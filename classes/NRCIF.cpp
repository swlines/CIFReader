/**

    CIF Reader - parser of Network Rail and ATCO-CIF files.
    Copyright (C) 2011 Tom Cairns

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    
**/

// cif related stuff
#include "NRCIF.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/progress.hpp>
#include <mysql++.h>

using namespace std;
using namespace boost;

bool NRCIF::processFile(mysqlpp::Connection &conn, const char* filePath) {
	ifstream file;
	file.open(filePath, ifstream::in);
	
	// get the file size - probably a better way of doing this?!
	long begin, end, fileSize, fileCurrent;
	begin = file.tellg();
	file.seekg(0, ios::end);
	end = file.tellg();
	file.seekg(0, ios::beg);
	fileSize = (end - begin);
	fileCurrent = begin;
	long displayKb = fileSize / (long)1024;
		
	if(file.is_open()) {
		string line;
		
		getline(file, line);
		
		CIFRecord *record;		
		// we know that this will be the header, and in the record there is a try catch for mainframe id
		try {
			record = NRCIF::processLine(line);
		}
		catch(int i) {
			if(i == -1) { 
				cerr << "The header file does not represent a valid CIF." << endl;
			}
			else {
				cerr << "Unknown error occured. Exiting..." << endl;
			}
			
			conn.disconnect();
			return false;
		}
		
		CIFRecordNRHD *header; 
		
		if(record->getRecordType() == 00) {
			header = (CIFRecordNRHD *)record;
		}
		else {
			delete record;
			cerr << "Unable to process " << filePath << " as it is not a valid CIF" << endl;
			conn.disconnect();
			return false;
		}
		
		cout << "===========================================================" << endl;
		cout << "Performing run on Network Rail CIF: " << filePath << endl;
		cout << "Importing " << header->curr_file_ref << " created for " << header->mainframe_user << endl;
		cout << "Generated on " << header->date_extract << " at " << header->time_extract << endl;
		cout << "Data from " << header->extract_start << " to " << header->extract_end << endl;
		cout << "File type: ";
		
		if(header->update_type == "F") { cout << "FULL; "; } else { cout << "UPDATE;"; }
		cout << " file size; " << displayKb << " kilobytes" << endl;
		
		cout << "===========================================================" << endl;
		
		progress_display progBar(fileSize, cout);
		
		mysqlpp::Query query = conn.query();
		
		// tiplocs
		vector<tiplocs_t> tiplocInsert;
		vector<string> tiplocDelete;
		bool tiplocComplete = false;
		
		// associations
		vector<associations_t> associationInsert;
		vector<string> associationDelete;
		bool associationComplete = false;
		
		vector<schedules_t> scheduleInsert;
		vector<locations_t> locationInsert;
		vector<locations_change_t> locationsChangeInsert;
		vector<string> scheduleDelete;
		unsigned scheduleInsNo = 0;
		
		while(getline(file, line)) {
		try{
			record = NRCIF::processLine(line);
			unsigned recordType = record->getRecordType();
			
			if(recordType == 1 || recordType == 2 || recordType == 3) { // tiploc insert or amend
				CIFRecordNRTITA *tiplocRecord = (CIFRecordNRTITA *)record;
				
				if(recordType == 2 || recordType == 3) {
					if(header->update_type == "F") {
						cerr << endl << "ERROR: TIPLOC AMEND record appeared in a full update. Exiting." << endl;
						delete record;
						conn.disconnect();
						return false;
					}
					
					if(recordType == 2) {
						tiplocDelete.push_back(tiplocRecord->tiploc_code);
					}
					else if(recordType == 3) {
						tiplocDelete.push_back(tiplocRecord->old_tiploc);
					}
				}
				
				tiplocInsert.push_back(tiplocs_t(tiplocRecord->tiploc_code, tiplocRecord->nlc, tiplocRecord->tps_desc, tiplocRecord->stanox, tiplocRecord->crs, tiplocRecord->capri_desc));
			}
			else if(recordType == 4) { // tiploc delete
				CIFRecordNRTD *tiplocRecord = (CIFRecordNRTD *)record;
				
				if(header->update_type != "F") {
					tiplocDelete.push_back(tiplocRecord->tiploc_code);
				}
				else if(header->update_type == "F") {
					cerr << endl << "ERROR: TIPLOC DELETE record appeared in a full update. Exiting." << endl;
					delete record;
					conn.disconnect();
					return false;
				}
			}
			else if(recordType == 5) { // associations
				// process it...
				CIFRecordNRAA *associationDetail = (CIFRecordNRAA *)record;
				
				if(associationDetail->transaction_type == "R" || associationDetail->transaction_type == "D") {
					if(header->update_type == "F") {
						cerr << endl << "ERROR: ASSOCIATION REVISE/DELETE record appeared in a full update. Exiting." << endl;
						delete record;
						conn.disconnect();
						return false;
					}
					
					if(associationDetail->stp_indicator != "C") {
						// find this association and remove it.
						string uuid;
						
						try {
							uuid = NRCIF::findUUIDForAssociation(conn, associationDetail, true, true, false);
						}
						catch(int e){
							cerr << endl << "ERROR: Unable to locate association to delete/amend. Exiting." << endl;
							delete record;
							conn.disconnect();
							return false;
						}
						
						associationDelete.push_back(uuid);
						uuid.clear();
					}
					else if(associationDetail->stp_indicator == "C" && associationDetail->transaction_type == "D") {
						// cancelling the cancellation of an LTP service... (i wish i could fit more cancels in here :))
						
						string uuid;
						
						// find the permament service relating to these dates
						try {
							uuid = NRCIF::findUUIDForAssociation(conn, associationDetail, false, true, false);
						}
						catch(int e) {
							cerr << "ERROR: Unable to locate association to remove STP cancel" << endl;
							delete record;
							conn.disconnect();
							return false;
						}
						
						NRCIF::deleteSTPAssociationCancellation(conn, uuid, associationDetail->date_from, associationDetail->date_to);
						uuid.clear();
					}
				}
				
				if(associationDetail->transaction_type == "N" || associationDetail->transaction_type == "R") {
					if(associationDetail->stp_indicator != "C") {
						// we have all the information we need anyway from this... so just need
						// to put it in the database
						

						associationInsert.push_back(associations_t(associationDetail->unique_id,
											 associationDetail->main_train_uid,
											 associationDetail->assoc_train_uid,
											 mysqlpp::sql_date(associationDetail->date_from),
											 mysqlpp::sql_date(associationDetail->date_to),
											 associationDetail->assoc_mo,
											 associationDetail->assoc_tu,
											 associationDetail->assoc_we,
											 associationDetail->assoc_th,
											 associationDetail->assoc_fr,
											 associationDetail->assoc_sa,
											 associationDetail->assoc_su,
											 associationDetail->category,
											 associationDetail->date_indicator,
											 associationDetail->location,
											 associationDetail->base_location_suffix,
											 associationDetail->assoc_location_suffix,
											 associationDetail->assoc_type,
											 associationDetail->stp_indicator));
					}
					else if(associationDetail->stp_indicator == "C") {
						// this is a LTP cancellation ... 
						// locate the uuid of the service being cancelled...
						
						// get uuid of service being STP cancelled
						string uuid;
						try {
							uuid = NRCIF::findUUIDForAssociation(conn, associationDetail, false, true, false);
						}
						catch(int e) {
							cerr << "ERROR: Unable to locate association to STP cancel" << endl;
							delete record;
							conn.disconnect();
							return false;
						}
						
						mysqlpp::Query query = conn.query();
						
						associations_stpcancel_t row(uuid, 
												   mysqlpp::sql_date(associationDetail->date_from), 
												   mysqlpp::sql_date(associationDetail->date_to),
												   associationDetail->assoc_mo,
												   associationDetail->assoc_tu,
												   associationDetail->assoc_we,
												   associationDetail->assoc_th,
												   associationDetail->assoc_fr,
												   associationDetail->assoc_sa,
												   associationDetail->assoc_su);
						
						query.insert(row);
						query.execute();
						uuid.clear();
					}
				}
			}
			else if(recordType == 6) { // insert
				// just finish any tiploc stuff... done here as associations aren't guaranteed
				if(!tiplocComplete) {
					NRCIF::runTiploc(conn, tiplocInsert, tiplocDelete);
					tiplocComplete = true;
				}
				
				if(!associationComplete) {
					NRCIF::runAssociation(conn, associationInsert, associationDelete);
					associationComplete = true;
				}
				
				if(scheduleInsNo > 20000) {
					NRCIF::runSchedules(conn, scheduleInsert, locationInsert, locationsChangeInsert, scheduleDelete);
					scheduleInsNo = 0;
				}
			
				CIFRecordNRBS *scheduleDetail = (CIFRecordNRBS *)record;
				
				if(scheduleDetail->transaction_type == "R" || scheduleDetail->transaction_type == "D") {
					if(header->update_type == "F") {
						cerr << endl << "ERROR: SCHEDULE REVISE/DELETE record appeared in a full update. Exiting." << endl;
						delete record;
						conn.disconnect();
						return false;
					}
					
					if(scheduleDetail->stp_indicator != "C") { // deletion of a non STP cancel.
						string uuid;
					
						try {
							uuid = NRCIF::findUUIDForService(conn, scheduleDetail, true, false, false);
						}
						catch(int e) {
							cerr << "ERROR: Unable to locate service to delete/amend. Exiting." << endl;
							delete record;
							conn.disconnect();
							return false;
						}
					
						scheduleDelete.push_back(uuid);
						uuid.clear();
						scheduleInsNo++;
					}
					else if(scheduleDetail->stp_indicator == "C" && scheduleDetail->transaction_type == "D") {
						// deletion of an STP cancel...
						string uuid;
						
						// find the permament service relating to these dates
						try {
							uuid = NRCIF::findUUIDForService(conn, scheduleDetail, false, true, false);
						}
						catch(int e) {
							cerr << "ERROR: Unable to locate service to remove STP cancel" << endl;
							delete record;
							conn.disconnect();
							return false;
						}
						
						NRCIF::deleteSTPServiceCancellation(conn, uuid, scheduleDetail->date_from, scheduleDetail->date_to);
						uuid.clear();
					}
				}
									
				if(scheduleDetail->transaction_type == "N" || scheduleDetail->transaction_type == "R") {
					if(scheduleDetail->stp_indicator != "C") { 
					
						// the BX record is always the next line, so just parse this straight back to the scheduleDetail.
						getline(file, line);
						scheduleDetail->mergeWithBX(line);
					
					// ok set it up and now get schedule locations
						vector<locations_t> locs;
						vector<locations_change_t> locs_change;
						
						CIFRecord *schedulerec;
						int location_order = 0;
						
						while(1) {
							getline(file, line);
							try {
								schedulerec = NRCIF::processLine(line);
							} 
							catch(int e) { continue; } // unknown line, would cause segmentation fault
							
							if(schedulerec->getRecordType() == 7) {
								CIFRecordNRLOLILT *location = (CIFRecordNRLOLILT *)schedulerec;
								
								locationInsert.push_back(locations_t(scheduleDetail->unique_id, location_order, location->record_type, location->tiploc, location->tiploc_instance, location->arrival, location->public_arrival, location->pass, location->departure, location->public_departure, location->platform, location->line, location->path, location->engineering_allowance, location->pathing_allowance, location->performance_allowance, location->activity));
							
								if(location->record_type == "LT") { delete schedulerec; break; }
								
								location_order++;
								scheduleInsNo++;
							}
							else if(schedulerec->getRecordType() == 8) {
								CIFRecordNRCR *cr = (CIFRecordNRCR *)schedulerec;
								
								locationsChangeInsert.push_back(locations_change_t(scheduleDetail->unique_id, 
																	   cr->tiploc,
																	   cr->tiploc_suffix,
																	   cr->category,
																	   cr->train_identity,
																	   cr->headcode,
																	   cr->service_code,
																	   cr->portion_id,
																	   cr->power_type,
																	   cr->timing_load,
																	   cr->speed,
																	   cr->operating_characteristics,
																	   cr->train_class,
																	   cr->sleepers,
																	   cr->reservations,
																	   cr->catering_code,
																	   cr->service_branding,
																	   cr->uic_code,
																	   cr->rsid));
								scheduleInsNo++;
							}
							
							delete schedulerec;
						}
						
						try {
							mysqlpp::Query query = conn.query();
							
							// load up the schedule
							scheduleInsert.push_back(schedules_t(scheduleDetail->unique_id,
										  scheduleDetail->uid,
										  mysqlpp::sql_date(scheduleDetail->date_from),
										  mysqlpp::sql_date(scheduleDetail->date_to),
										  scheduleDetail->runs_mo,
										  scheduleDetail->runs_tu,
										  scheduleDetail->runs_we,
										  scheduleDetail->runs_th,
										  scheduleDetail->runs_fr,
										  scheduleDetail->runs_sa,
										  scheduleDetail->runs_su,
										  scheduleDetail->bank_holiday,
										  scheduleDetail->status,
										  scheduleDetail->category,
										  scheduleDetail->train_identity,
										  scheduleDetail->headcode,
										  scheduleDetail->service_code,
										  scheduleDetail->portion_id,
										  scheduleDetail->power_type,
										  scheduleDetail->timing_load,
										  scheduleDetail->speed,
										  scheduleDetail->operating_characteristics,
										  scheduleDetail->train_class,
										  scheduleDetail->sleepers,
										  scheduleDetail->reservations,
										  scheduleDetail->catering_code,
										  scheduleDetail->service_branding,
										  scheduleDetail->stp_indicator,
										  scheduleDetail->uic_code,
										  scheduleDetail->atoc_code,
										  scheduleDetail->ats_code,
										  scheduleDetail->rsid,
										  scheduleDetail->data_source));
										  
							scheduleInsNo++;
						}
						catch(mysqlpp::BadQuery& e) {
							cerr << "Error (query): " << e.what() << endl;
							conn.disconnect();
							return false;
						}
					}
					else if(scheduleDetail->stp_indicator == "C") {
						// this is a STP CAN of LTP schedule
						// locate the uuid of the service being cancelled...
						
						// get uuid of service being STP cancelled
						string uuid;
						try {
							uuid = NRCIF::findUUIDForService(conn, scheduleDetail, false, true, false);
						}
						catch(int e) {
							cerr << "ERROR: Unable to locate service to STP cancel" << endl;
							delete record;
							conn.disconnect();
							return false;
						}
						
						mysqlpp::Query query = conn.query();
						
						schedules_stpcancel_t row(uuid, 
												mysqlpp::sql_date(scheduleDetail->date_from), 
												mysqlpp::sql_date(scheduleDetail->date_to),
												scheduleDetail->runs_mo,
												scheduleDetail->runs_tu,
												scheduleDetail->runs_we,
												scheduleDetail->runs_th,
												scheduleDetail->runs_fr,
												scheduleDetail->runs_sa,
												scheduleDetail->runs_su);
						
						query.insert(row);
						query.execute();
						uuid.clear();
					} 
				}
			}
			
		}catch(int e){ continue; } 
		delete record; 
		
		progBar += (long)((long)(file.tellg()) - fileCurrent);
		fileCurrent = file.tellg();
		}
		
		if(scheduleInsNo > 0) {
			NRCIF::runSchedules(conn, scheduleInsert, locationInsert, locationsChangeInsert, scheduleDelete);
		}
		
		cout << endl << "INFO: File complete..." << endl;
		
		if(!tiplocComplete) {
			cout << "INFO: Processing TIPLOCs that weren't completed during run." << endl;
			NRCIF::runTiploc(conn, tiplocInsert, tiplocDelete);
			tiplocComplete = true;
		}
		
		if(!associationComplete) {
			cout << "INFO: Processing associations that weren't completed during run." << endl;
			NRCIF::runAssociation(conn, associationInsert, associationDelete);
			associationComplete = true;
		}
		
		delete header;
	}
	else {
		cout << "ERROR: Cannot open file (" << filePath << "), moving on..." << endl;
		file.close();
		return false;
	}
	
	cout << endl;
	
	file.close();
	return true;
}

CIFRecord* NRCIF::processLine(string record) {
	string recordType;
	recordType = record.substr(0,2);
	
	if(recordType == "HD") { // header
		return new CIFRecordNRHD(record);
	}
		
	else if(recordType == "TI" || recordType == "TA") { // tiploc insert
		return new CIFRecordNRTITA(record);	
	}
	else if(recordType == "TD") { // tiploc delete
		return new CIFRecordNRTD(record);
	} 	
	
	else if(recordType == "BS") { // new schedule
		return new CIFRecordNRBS(record);	
	}	
	else if(recordType == "LO" || recordType == "LI" || recordType == "LT") { // journey origin
		return new CIFRecordNRLOLILT(record);
	}
	else if(recordType == "CR") {
		return new CIFRecordNRCR(record);
	}
	
	else if(recordType == "AA") {
		return new CIFRecordNRAA(record);
	}
	
	else {
		throw 1;
	}
}

void NRCIF::runTiploc(mysqlpp::Connection &conn, vector<tiplocs_t> &tiplocInsert, vector<string> &tiplocDelete) {
	mysqlpp::Query query = conn.query();

	// delete the old tiplocs
	vector<string>::iterator dit;
	for(dit = tiplocDelete.begin(); dit < tiplocDelete.end(); dit++) {
		query << "DELETE FROM tiplocs WHERE tiploc = " << mysqlpp::quote << *dit;
		query.execute();
	}
	tiplocDelete.clear();

	// insert the new tiplocs
	vector<tiplocs_t>::iterator iit;
	for(iit = tiplocInsert.begin(); iit < tiplocInsert.end(); iit++) {
		try {
			query.insert(*iit);
			query.execute();
		}
		catch(mysqlpp::BadQuery& e) {}
	}
	tiplocInsert.clear();
}

void NRCIF::runAssociation(mysqlpp::Connection &conn, vector<associations_t> &associationInsert, vector<string> &associationDelete) {
	mysqlpp::Query query = conn.query();
	
	// delete the old associations
	vector<string>::iterator dit;
	for(dit = associationDelete.begin(); dit < associationDelete.end(); dit++) {
		query << "DELETE FROM associations WHERE uuid = " << mysqlpp::quote << *dit;
		query.execute();
	}
	associationDelete.clear();

	// insert the new associations
	vector<associations_t>::iterator iit;
	for(iit = associationInsert.begin(); iit < associationInsert.end(); iit++) {
		try {
			query.insert(*iit);
			query.execute();
		}
		catch(mysqlpp::BadQuery& e) {}
	}
	associationInsert.clear();
}

void NRCIF::runSchedules(mysqlpp::Connection &conn, vector<schedules_t> &scheduleInsert, vector<locations_t> &locationInsert, vector<locations_change_t> &locationsChangeInsert, vector<string> &scheduleDelete) {
	mysqlpp::Query query = conn.query();
	
	// delete the old locations
	vector<string>::iterator dit;
	for(dit = scheduleDelete.begin(); dit < scheduleDelete.end(); dit++) {
		NRCIF::deleteService(conn, *dit);
	}
	scheduleDelete.clear();
	
	mysqlpp::Query::SizeThresholdInsertPolicy<> insert_policy(5000);
	query.insertfrom(scheduleInsert.begin(), scheduleInsert.end(), insert_policy);
	scheduleInsert.clear();
	
	query.insertfrom(locationInsert.begin(), locationInsert.end(), insert_policy);
	locationInsert.clear();
	
	query.insertfrom(locationsChangeInsert.begin(), locationsChangeInsert.end(), insert_policy);
	locationsChangeInsert.clear();
}

string NRCIF::findUUIDForService(mysqlpp::Connection &conn, CIFRecordNRBS *s, bool exact, bool removeDoesntRunOn, bool noDateTo) {
	mysqlpp::Query query = conn.query();
	
	// create string to check runs on dates...
	string runs_on = "";
	if(removeDoesntRunOn) {
		if(s->runs_mo != "" && s->runs_mo != "0") runs_on += " AND runs_mo = '" + s->runs_mo + "'";
		if(s->runs_tu != "" && s->runs_tu != "0") runs_on += " AND runs_tu = '" + s->runs_tu + "'";
		if(s->runs_we != "" && s->runs_we != "0") runs_on += " AND runs_we = '" + s->runs_we + "'";
		if(s->runs_th != "" && s->runs_th != "0") runs_on += " AND runs_th = '" + s->runs_th + "'";
		if(s->runs_fr != "" && s->runs_fr != "0") runs_on += " AND runs_fr = '" + s->runs_fr + "'";
		if(s->runs_sa != "" && s->runs_sa != "0") runs_on += " AND runs_sa = '" + s->runs_sa + "'";
		if(s->runs_su != "" && s->runs_su != "0") runs_on += " AND runs_su = '" + s->runs_su + "'";
	}
	else {
		if(s->runs_mo != "") runs_on += " AND runs_mo = '" + s->runs_mo + "'";
		if(s->runs_tu != "") runs_on += " AND runs_tu = '" + s->runs_tu + "'";
		if(s->runs_we != "") runs_on += " AND runs_we = '" + s->runs_we + "'";
		if(s->runs_th != "") runs_on += " AND runs_th = '" + s->runs_th + "'";
		if(s->runs_fr != "") runs_on += " AND runs_fr = '" + s->runs_fr + "'";
		if(s->runs_sa != "") runs_on += " AND runs_sa = '" + s->runs_sa + "'";
		if(s->runs_su != "") runs_on += " AND runs_su = '" + s->runs_su + "'";
	}
	
	// find this service
	if(exact) {
		if(s->date_to != "" && !noDateTo) {
			query << "SELECT uuid FROM schedules_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND date_from = " << mysqlpp::quote << s->date_from << " AND date_to = " << mysqlpp::quote << s->date_to << " " << runs_on << " AND stp_indicator = " << mysqlpp:quote <<  s->stp_indicator << " LIMIT 0,1";
		}
		else {
			query << "SELECT uuid FROM schedules_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND date_from = " << mysqlpp::quote << s->date_from << " " << runs_on << " AND stp_indicator = " << mysqlpp:quote <<  s->stp_indicator << " LIMIT 0,1";
		}
	}
	else {
		if(s->date_to != "" && !noDateTo) {
			query << "SELECT uuid FROM schedules_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND (" << mysqlpp::quote << s->date_from << " BETWEEN date_from AND date_to) AND (" << mysqlpp::quote << s->date_to << " BETWEEN date_from AND date_to) " <<  runs_on << " LIMIT 0,1"; 
		}
		else {
			query << "SELECT uuid FROM schedules_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND (" << mysqlpp::quote << s->date_from << " BETWEEN date_from AND date_to) " << runs_on << " LIMIT 0,1";
		}
	}
	
	string queryString = query.str();
		
	if(mysqlpp::StoreQueryResult res = query.store()) {
		if(res.num_rows() > 0) {
			return res[0]["uuid"].c_str();
		}
		
		if(!noDateTo)
			return NRCIF::findUUIDForService(conn, s, exact, removeDoesntRunOn, true);
		else if(noDateTo && !removeDoesntRunOn) 
			return NRCIF::findUUIDForService(conn, s, exact, true, true);
		else {
			cout << endl << "Error query: " << queryString << endl;
			throw 1;
		}
	}
	
	throw 2;
}



void NRCIF::deleteService(mysqlpp::Connection &conn, string uuid) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM locations_t WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
	
	query << "DELETE FROM locations_change_t WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
	
	query << "DELETE FROM schedules_t WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
	
	query << "DELETE FROM schedules_stpcancel_t WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
}

void NRCIF::deleteSTPServiceCancellation(mysqlpp::Connection &conn, string uuid, string cancelFrom, string cancelTo) {
	mysqlpp::Query query = conn.query();
	
	if(cancelTo == "") {
		query << "DELETE FROM schedules_stpcancel_t WHERE uuid = " << mysqlpp::quote << uuid << " AND cancel_from = " << mysqlpp::quote << cancelFrom;
	}
	else {
		query << "DELETE FROM schedules_stpcancel_t WHERE uuid = " << mysqlpp::quote << uuid << " AND cancel_from = " << mysqlpp::quote << cancelFrom << " AND cancel_to = " << mysqlpp::quote << cancelTo;
	}
	
	query.execute();
}

string NRCIF::findUUIDForAssociation(mysqlpp::Connection &conn, CIFRecordNRAA *a, bool exact, bool removeDoesntRunOn, bool noDateTo) {
	mysqlpp::Query query = conn.query(); 
	
	// create string to check assoc on dates...
	string assoc_on = "";
	if(removeDoesntRunOn) {
		if(a->assoc_mo != "" && a->assoc_mo != "0") assoc_on += " AND assoc_mo = '" + a->assoc_mo + "'";
		if(a->assoc_tu != "" && a->assoc_tu != "0") assoc_on += " AND assoc_tu = '" + a->assoc_tu + "'";
		if(a->assoc_we != "" && a->assoc_we != "0") assoc_on += " AND assoc_we = '" + a->assoc_we + "'";
		if(a->assoc_th != "" && a->assoc_th != "0") assoc_on += " AND assoc_th = '" + a->assoc_th + "'";
		if(a->assoc_fr != "" && a->assoc_fr != "0") assoc_on += " AND assoc_fr = '" + a->assoc_fr + "'";
		if(a->assoc_sa != "" && a->assoc_sa != "0") assoc_on += " AND assoc_sa = '" + a->assoc_sa + "'";
		if(a->assoc_su != "" && a->assoc_su != "0") assoc_on += " AND assoc_su = '" + a->assoc_su + "'";
	}
	else {
		if(a->assoc_mo != "") assoc_on += " AND assoc_mo = '" + a->assoc_mo + "'";
		if(a->assoc_tu != "") assoc_on += " AND assoc_tu = '" + a->assoc_tu + "'";
		if(a->assoc_we != "") assoc_on += " AND assoc_we = '" + a->assoc_we + "'";
		if(a->assoc_th != "") assoc_on += " AND assoc_th = '" + a->assoc_th + "'";
		if(a->assoc_fr != "") assoc_on += " AND assoc_fr = '" + a->assoc_fr + "'";
		if(a->assoc_sa != "") assoc_on += " AND assoc_sa = '" + a->assoc_sa + "'";
		if(a->assoc_su != "") assoc_on += " AND assoc_su = '" + a->assoc_su + "'";
	}
	
	// find this association
	if(exact) {
		if(a->date_to != "" && !noDateTo) {
			query << "SELECT uuid FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND date_from = " << mysqlpp::quote << a->date_from << " AND date_to = " << mysqlpp::quote << a->date_to << assoc_on << " AND stp_indicator = " << mysqlpp:quote <<  s->stp_indicator << "  LIMIT 0,1"; 
		}
		else {
			query << "SELECT uuid FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND date_from = " << mysqlpp::quote << a->date_from << assoc_on << " AND stp_indicator = " << mysqlpp:quote <<  s->stp_indicator << " LIMIT 0,1";
		}
	}
	else{
		if(a->date_to != "" && !noDateTo) {
			query << "SELECT uuid FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND (" << mysqlpp::quote << a->date_from << " BETWEEN date_from AND date_to) AND (" << mysqlpp::quote << a->date_to << " BETWEEN date_from AND date_to) " << assoc_on << " LIMIT 0,1"; 
		}
		else {
			query << "SELECT uuid FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND (" << mysqlpp::quote << a->date_from << " BETWEEN date_from AND date_to) " << assoc_on << " LIMIT 0,1";
		}
	}
	
	string queryString = query.str();
	
	if(mysqlpp::StoreQueryResult res = query.store()) {
		if(res.num_rows() > 0) {
			return res[0]["uuid"].c_str();
		}
		
		if(!noDateTo)
			return NRCIF::findUUIDForAssociation(conn, a, exact, removeDoesntRunOn, true);
		else if(noDateTo && !removeDoesntRunOn) 
			return NRCIF::findUUIDForAssociation(conn, a, exact, true, true);
		else {
			cout << endl << "Error query: " << queryString << endl;
			throw 1;
		}
	}
	
	throw 2;
}

void NRCIF::deleteAssociation(mysqlpp::Connection &conn, string uuid) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM associations_t WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
	
	query << "DELETE FROM associations_stpcancel_t WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
}


void NRCIF::deleteSTPAssociationCancellation(mysqlpp::Connection &conn, string uuid, string cancelFrom, string cancelTo) {
	mysqlpp::Query query = conn.query();
	
	if(cancelTo == "") {
		query << "DELETE FROM associations_stpcancel_t WHERE uuid = " << mysqlpp::quote << uuid << " AND cancel_from = " << mysqlpp::quote << cancelFrom;
	}
	else {
		query << "DELETE FROM associations_stpcancel_t WHERE uuid = " << mysqlpp::quote << uuid << " AND cancel_from = " << mysqlpp::quote << cancelFrom << " AND cancel_to = " << mysqlpp::quote << cancelTo;
	}
	
	query.execute();
}
