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

#ifndef _SQLNETRAIL_
	#define _SQLNETRAIL_ 1
	#include "sqlNetRail.h"
#endif

// cif related stuff
#ifndef _NRCIF_INC_
	#define _NRCIF_INC_
	#include "NRCIF.h"
#endif
	
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/progress.hpp>
#include <mysql++.h>

using namespace std;
using namespace boost;

/** hack for mysql++ **/
class NR_CIF {
	public: 
		static void runAssociation(mysqlpp::Connection &conn, vector<associations_t> &associationInsert, vector<int> &associationDelete);
		static void runSchedules(mysqlpp::Connection &conn, vector<locations_t> &locationInsert, vector<locations_change_t> &locationsChangeInsert, vector<int> &scheduleDelete);
};

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
		
		updates updaterow(header->curr_file_ref, header->mainframe_user, mysqlpp::sql_date(header->extract_start), mysqlpp::sql_date(header->extract_end), header->update_type);
		
		mysqlpp::Query query = conn.query();
		
		// tiplocs
		vector<CIFRecordNRTITA *> tiplocInsert;
		vector<string> tiplocDelete;
		bool tiplocComplete = false;
		
		// associations
		vector<associations_t> associationInsert;
		vector<int> associationDelete;
		bool associationComplete = false;
		
		vector<schedules_t> scheduleInsert;
		vector<locations_t> locationInsert;
		vector<locations_change_t> locationsChangeInsert;
		vector<int> scheduleDelete;
		unsigned scheduleInsNo = 0;
		
		vector<CIFRecordNRAA *> associationSTPCancelDelete;
		vector<CIFRecordNRAA *> associationSTPCancelInsert;
		vector<CIFRecordNRBS *> scheduleSTPCancelDelete;
		vector<CIFRecordNRBS *> scheduleSTPCancelInsert;
		
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
				
				tiplocInsert.push_back(tiplocRecord);
				continue;
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
				int id;
				
				if(associationDetail->transaction_type == "R" || associationDetail->transaction_type == "D") {
					if(header->update_type == "F") {
						cerr << endl << "ERROR: ASSOCIATION REVISE/DELETE record appeared in a full update. Exiting." << endl;
						delete record;
						conn.disconnect();
						return false;
					}
					
					if(associationDetail->stp_indicator != "C") {
						// find this association and remove it.
						try {
							id = NRCIF::findIDForAssociation(conn, associationDetail, header, true, true, false);
						}
						catch(int e){
							cerr << endl << "ERROR: Unable to locate association to delete/amend (main UID " << associationDetail->main_train_uid << ", assoc UID " << associationDetail->assoc_train_uid << "). Exiting." << endl;
							delete record;
							conn.disconnect();
							return false;
						}
						
						while(id < 0) {
							NRCIF::deleteAssociation(conn, -id);
							id = NRCIF::findIDForAssociation(conn, associationDetail, header, true, true, false);
						}
						
						if(associationDetail->transaction_type == "D") {
							associationDelete.push_back(id);
						} else if(associationDetail->transaction_type == "R") {
							NRCIF::deleteAssociation(conn, id);
						}
					}
					else if(associationDetail->stp_indicator == "C") {
						// cancelling the cancellation of an LTP service... (i wish i could fit more cancels in here :))
						
						// deletion of an STP cancel...
						// push back into vector
						associationSTPCancelDelete.push_back(associationDetail);
						if(associationDetail->transaction_type == "D") {
							continue; // so it doesn't get deleted
						}
					}
				}
				
				if(associationDetail->transaction_type == "N" || associationDetail->transaction_type == "R") {
					if(associationDetail->stp_indicator != "C") {
						// we have all the information we need anyway from this... so just need
						// to put it in the database
						
						// i'm aware of an error in CIF data where we can get more than one association that is essentially the same
						// that can cause an issue upstream, so to resolve this...run a quick check
						// in this instance, I *expect* an exception to be thrown. This trace has already been done for 
						// associations when under revision, so only cover new schedules here...
						if(header->update_type == "U" && associationDetail->transaction_type == "N") {
							try {
								id = NRCIF::findIDForAssociation(conn, associationDetail, header, true, false, false);
								while(id < 0) {
									NRCIF::deleteAssociation(conn, -id);
									id = NRCIF::findIDForAssociation(conn, associationDetail, header, true, false, false);
								}
								
								NRCIF::deleteAssociation(conn, id);
								
								// as we have just deleted something, to maintain historical cancellations make sure that this is a revise
								associationDetail->transaction_type = "R";
							}catch(int e){}
						}
						
						if(associationDetail->transaction_type == "R") {
							associationInsert.push_back(associations_t(associationDetail->main_train_uid,
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
												 associationDetail->stp_indicator,
												 id));
						} else {
							associationInsert.push_back(associations_t(associationDetail->main_train_uid,
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
												 associationDetail->stp_indicator,
												 mysqlpp::null));
						}
					}
					else if(associationDetail->stp_indicator == "C") {
						// this is a LTP cancellation ... 
						// locate the id of the service being cancelled...						
						
						// load this record up into the vector
						associationSTPCancelInsert.push_back(associationDetail);
						
						continue; // to stop record deletion
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
					NR_CIF::runAssociation(conn, associationInsert, associationDelete);
					NRCIF::runAssociationsStpCancel(conn, header, associationSTPCancelDelete, associationSTPCancelInsert);
					associationComplete = true;
				}
				
				if(scheduleInsNo > 40000) {
					NR_CIF::runSchedules(conn, locationInsert, locationsChangeInsert, scheduleDelete);
					scheduleInsNo = 0;
				}
							
				CIFRecordNRBS *scheduleDetail = (CIFRecordNRBS *)record;
				int id;
				
				if(scheduleDetail->transaction_type == "R" || scheduleDetail->transaction_type == "D") {
					if(header->update_type == "F") {
						cerr << endl << "ERROR: SCHEDULE REVISE/DELETE record appeared in a full update. Exiting." << endl;
						delete record;
						conn.disconnect();
						return false;
					}
					
					if(scheduleDetail->stp_indicator != "C") { // deletion of a non STP cancel.
						try {
							id = NRCIF::findIDForService(conn, scheduleDetail, header, true, false, false);
						}
						catch(int e) {
							cerr << "ERROR: Unable to locate service to delete/amend ( " << scheduleDetail->uid << "). Exiting." << endl;
							delete record;
							conn.disconnect();
							return false;
						}
						
						while(id < 0) {
							NRCIF::deleteService(conn, -id);
							id = NRCIF::findIDForService(conn, scheduleDetail, header, true, false, false);
						}
					
						if(scheduleDetail->transaction_type == "R") {
							NRCIF::deleteService(conn, id);
						} else if(scheduleDetail->transaction_type == "D") {
							scheduleDelete.push_back(id);
							scheduleInsNo++;
						}
					}
					else if(scheduleDetail->stp_indicator == "C") {
						// deletion of an STP cancel...
						// push back into vector
						scheduleSTPCancelDelete.push_back(scheduleDetail);
						scheduleInsNo++;
						if(scheduleDetail->transaction_type == "D") {
							continue; // so it doesn't get deleted
						}
					}
				}
									
				if(scheduleDetail->transaction_type == "N" || scheduleDetail->transaction_type == "R") {
					if(scheduleDetail->stp_indicator != "C") { 
					
						// the BX record is always the next line, so just parse this straight back to the scheduleDetail.
						getline(file, line);
						scheduleDetail->mergeWithBX(line);
						
						// i'm aware of an error in CIF data where we can get more than one unique id (that is, <uid><start><stp>)
						// that can cause an issue upstream, so to resolve this...run a quick check
						// in this instance, I *expect* an exception to be thrown. This trace has already been done for 
						// schedules when under revision, so only cover new schedules here...
						if(header->update_type == "U" && scheduleDetail->transaction_type == "N") {
							try {
								id = NRCIF::findIDForService(conn, scheduleDetail, header, true, false, false);
								while(id < 0) {
									NRCIF::deleteService(conn, -id);
									id = NRCIF::findIDForService(conn, scheduleDetail, header, true, false, false);
								}
								
								NRCIF::deleteService(conn, id);
								// as we have just deleted something, to maintain historical cancellations make sure that this is a revise
								scheduleDetail->transaction_type = "R";
							}catch(int e){}
						}
					
					// ok set it up and now get schedule locations
						vector<locations_t> locs;
						vector<locations_change_t> locs_change;
						
						CIFRecord *schedulerec;
						int location_order = 0;
						unsigned scheduleId = 0;
						
						try {
							mysqlpp::Query query = conn.query();
							
							// load up the schedule
							if(scheduleDetail->transaction_type == "R") {
								schedules_t row(scheduleDetail->uid,
											  scheduleDetail->unique_id,
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
											  scheduleDetail->data_source,
											  scheduleDetail->bus,
											  scheduleDetail->train,
											  scheduleDetail->ship,
											  scheduleDetail->passenger,
											  scheduleDetail->oc_b,
											  scheduleDetail->oc_c,
											  scheduleDetail->oc_d,
											  scheduleDetail->oc_e,
											  scheduleDetail->oc_g,
											  scheduleDetail->oc_m,
											  scheduleDetail->oc_p,
											  scheduleDetail->oc_q,
											  scheduleDetail->oc_r,
											  scheduleDetail->oc_s,
											  scheduleDetail->oc_y,
											  scheduleDetail->oc_z,
											  id);
											  
								query.insert(row);
								
							} else { 
								schedules_t row(scheduleDetail->uid,
											  scheduleDetail->unique_id,
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
											  scheduleDetail->data_source,
											  scheduleDetail->bus,
											  scheduleDetail->train,
											  scheduleDetail->ship,
											  scheduleDetail->passenger,
											  scheduleDetail->oc_b,
											  scheduleDetail->oc_c,
											  scheduleDetail->oc_d,
											  scheduleDetail->oc_e,
											  scheduleDetail->oc_g,
											  scheduleDetail->oc_m,
											  scheduleDetail->oc_p,
											  scheduleDetail->oc_q,
											  scheduleDetail->oc_r,
											  scheduleDetail->oc_s,
											  scheduleDetail->oc_y,
											  scheduleDetail->oc_z,
											  mysqlpp::null);
											  
								query.insert(row);
							}
										  
							query.execute();
							
							if(scheduleDetail->transaction_type == "N") {
								scheduleId = query.insert_id();
							} else {
								scheduleId = id;
							}
						}
						catch(mysqlpp::BadQuery& e) {
							cerr << "Error (query): " << e.what() << endl;
							conn.disconnect();
							return false;
						}
						
						while(1) {
							getline(file, line);
							try {
								schedulerec = NRCIF::processLine(line);
							} 
							catch(int e) { continue; } // unknown line, would cause segmentation fault
							
							if(schedulerec->getRecordType() == 7) {
								CIFRecordNRLOLILT *location = (CIFRecordNRLOLILT *)schedulerec;
								
								locationInsert.push_back(locations_t(scheduleId, 
								                                     location_order, 
								                                     location->record_type, 
								                                     location->tiploc, 
								                                     location->tiploc_instance, 
								                                     location->arrival, 
								                                     location->public_arrival, 
								                                     location->pass, 
								                                     location->departure, 
								                                     location->public_departure, 
								                                     location->platform, 
								                                     location->line, 
								                                     location->path, 
								                                     location->engineering_allowance, 
								                                     location->pathing_allowance, 
								                                     location->performance_allowance, 
								                                     location->activity, 
								                                     location->public_call, 
								                                     location->actual_call, 
								                                     location->order_time,
								                                     location->act_a,
								                                     location->act_ae,
								                                     location->act_bl,
								                                     location->act_c,
								                                     location->act_d,
								                                     location->act_minusd,
								                                     location->act_e,
								                                     location->act_g,
								                                     location->act_h,
								                                     location->act_hh,
								                                     location->act_k,
								                                     location->act_kc,
								                                     location->act_ke,
								                                     location->act_kf,
								                                     location->act_ks,
								                                     location->act_l,
								                                     location->act_n,
								                                     location->act_op,
								                                     location->act_or,
								                                     location->act_pr,
								                                     location->act_r,
								                                     location->act_rm,
								                                     location->act_rr,
								                                     location->act_s,
								                                     location->act_t,
								                                     location->act_minust,
								                                     location->act_tb,
								                                     location->act_tf,
								                                     location->act_ts,
								                                     location->act_tw,
								                                     location->act_u,
								                                     location->act_minusu,
								                                     location->act_w,
								                                     location->act_x));
							
								if(location->record_type == "LT") { delete schedulerec; break; }
								
								location_order++;
								scheduleInsNo++;
							}
							else if(schedulerec->getRecordType() == 8) {
								CIFRecordNRCR *cr = (CIFRecordNRCR *)schedulerec;
								
								locationsChangeInsert.push_back(locations_change_t(scheduleId, 
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
					}
					else if(scheduleDetail->stp_indicator == "C") {
						// this is a STP CAN of LTP schedule
						// locate the id of the service being cancelled...
						
						// load this record up into the vector
						scheduleSTPCancelInsert.push_back(scheduleDetail);
						
						continue; // to stop record deletion
					} 
				}
			}
			
		}catch(int e){ 
			progBar += (long)((long)(file.tellg()) - fileCurrent);
			fileCurrent = file.tellg();
			continue; 
		} 
		delete record; 
		
		progBar += (long)((long)(file.tellg()) - fileCurrent);
		fileCurrent = file.tellg();
		}
		
		NR_CIF::runSchedules(conn, locationInsert, locationsChangeInsert, scheduleDelete);
		NRCIF::runSchedulesStpCancel(conn, header, scheduleSTPCancelDelete, scheduleSTPCancelInsert);
		
		cout << endl << "INFO: File complete..." << endl;
		query.insert(updaterow);
		query.execute();
		
		
		if(!tiplocComplete) {
			cout << "INFO: Processing TIPLOCs that weren't completed during run." << endl;
			NRCIF::runTiploc(conn, tiplocInsert, tiplocDelete);
			tiplocComplete = true;
		}
		
		if(!associationComplete) {
			cout << "INFO: Processing associations that weren't completed during run." << endl;
			NR_CIF::runAssociation(conn, associationInsert, associationDelete);
			NRCIF::runAssociationsStpCancel(conn, header, associationSTPCancelDelete, associationSTPCancelInsert);
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

void NRCIF::runTiploc(mysqlpp::Connection &conn, vector<CIFRecordNRTITA *> &tiplocInsert, vector<string> &tiplocDelete) {
	mysqlpp::Query query = conn.query();

	// delete the old tiplocs
	vector<string>::iterator dit;
	for(dit = tiplocDelete.begin(); dit < tiplocDelete.end(); dit++) {
		query << "DELETE FROM tiplocs_t WHERE tiploc = " << mysqlpp::quote << *dit;
		query.execute();
	}
	tiplocDelete.clear();

	// insert the new tiplocs
	vector<tiplocs_t> tiplocs_ins;
	vector<CIFRecordNRTITA *>::iterator iit;
	for(iit = tiplocInsert.begin(); iit < tiplocInsert.end(); iit++) {
		tiplocs_ins.push_back(tiplocs_t((*iit)->tiploc_code, (*iit)->nlc, (*iit)->tps_desc, (*iit)->stanox, (*iit)->crs, (*iit)->capri_desc));
		delete *iit;
	}
	tiplocInsert.clear();
	
	mysqlpp::Query::SizeThresholdInsertPolicy<> insert_policy(5000);	
	query.insertfrom(tiplocs_ins.begin(), tiplocs_ins.end(), insert_policy);
	tiplocs_ins.clear();
}

void NR_CIF::runAssociation(mysqlpp::Connection &conn, vector<associations_t> &associationInsert, vector<int> &associationDelete) {
	mysqlpp::Query query = conn.query();
	
	// delete the old associations
	vector<int>::iterator dit;
	for(dit = associationDelete.begin(); dit < associationDelete.end(); dit++) {
		query << "DELETE FROM associations_t WHERE id = " << *dit;
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

void NR_CIF::runSchedules(mysqlpp::Connection &conn, vector<locations_t> &locationInsert, vector<locations_change_t> &locationsChangeInsert, vector<int> &scheduleDelete) {
	mysqlpp::Query query = conn.query();
	
	// delete the old locations
	vector<int>::iterator dit;
	for(dit = scheduleDelete.begin(); dit < scheduleDelete.end(); dit++) {
		NRCIF::deleteService(conn, *dit);
	}
	scheduleDelete.clear();
	
	mysqlpp::Query::SizeThresholdInsertPolicy<> insert_policy(5000);	
	query.insertfrom(locationInsert.begin(), locationInsert.end(), insert_policy);
	locationInsert.clear();
	
	query.insertfrom(locationsChangeInsert.begin(), locationsChangeInsert.end(), insert_policy);
	locationsChangeInsert.clear();
}

void NRCIF::runSchedulesStpCancel(mysqlpp::Connection &conn, CIFRecordNRHD *header, vector<CIFRecordNRBS *> &scheduleSTPCancelDelete, vector<CIFRecordNRBS *> &scheduleSTPCancelInsert) {
	mysqlpp::Query query = conn.query();
	
	CIFRecordNRBS *scheduleDetail;
	
	// run the schedule delete
	vector<CIFRecordNRBS *>::iterator dit;
	for(dit = scheduleSTPCancelDelete.begin(); dit < scheduleSTPCancelDelete.end(); dit++) {	
		scheduleDetail = *dit;
		
		// find the permament service relating to these dates
		try {
			NRCIF::deleteSTPServiceCancellation(conn, scheduleDetail);
		}
		catch(int e) {
			// we can assume this occurs when the service has already been deleted.
			// as CIF has no particular order, so move on...
		}
		
		if(scheduleDetail->transaction_type == "D") {
			delete scheduleDetail;
		}
	}
	scheduleSTPCancelDelete.clear();
	
	vector <CIFRecordNRBS *>::iterator iit;
	int id;
		
	for(iit = scheduleSTPCancelInsert.begin(); iit < scheduleSTPCancelInsert.end(); iit++) {
		scheduleDetail = *iit;
		
		// fix for variances in new/revise changes...
		if(header->update_type == "U" && scheduleDetail->transaction_type == "N") {
			try {
				id = NRCIF::findIDForServiceStpCancel(conn, scheduleDetail);
				while(id < 0) {
					NRCIF::deleteSTPServiceCancellation(conn, scheduleDetail);
					id = NRCIF::findIDForServiceStpCancel(conn, scheduleDetail);
				}
				
				NRCIF::deleteSTPServiceCancellation(conn, scheduleDetail);
			}catch(int e){}
		}
		
		schedules_stpcancel_core_t row(scheduleDetail->uid, 
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
								
		delete scheduleDetail;
	}
	scheduleSTPCancelInsert.clear();

}

void NRCIF::runAssociationsStpCancel(mysqlpp::Connection &conn, CIFRecordNRHD *header, vector<CIFRecordNRAA *> &associationSTPCancelDelete, vector<CIFRecordNRAA *> &associationSTPCancelInsert) {
	mysqlpp::Query query = conn.query();
	
	CIFRecordNRAA *associationDetail;
	
	// run the schedule delete
	vector<CIFRecordNRAA *>::iterator dit;
	for(dit = associationSTPCancelDelete.begin(); dit < associationSTPCancelDelete.end(); dit++) {	
		associationDetail = *dit;
							
		// find the permament service relating to these dates
		try {
			NRCIF::deleteSTPAssociationCancellation(conn, associationDetail);
		}
		catch(int e) {
			// we can assume this occurs when the service has already been deleted.
			// as CIF has no particular order, so move on...
		}
		
		if(associationDetail->transaction_type == "D") {
			delete associationDetail;
		}
	}
	associationSTPCancelDelete.clear();
	
	vector <CIFRecordNRAA *>::iterator iit;
	int id;
	
	for(iit = associationSTPCancelInsert.begin(); iit < associationSTPCancelInsert.end(); iit++) {
		associationDetail = *iit;
		
		// fix for variances in new/revise changes...
		if(header->update_type == "U" && associationDetail->transaction_type == "N") {
			try {
				id = NRCIF::findIDForAssociationStpCancel(conn, associationDetail);
				while(id < 0) {
					NRCIF::deleteSTPAssociationCancellation(conn, associationDetail);
					id = NRCIF::findIDForAssociationStpCancel(conn, associationDetail);
				}
				
				NRCIF::deleteSTPAssociationCancellation(conn, associationDetail);
			}catch(int e){}
		}
										
		associations_stpcancel_core_t row(associationDetail->main_train_uid, 
								   associationDetail->assoc_train_uid, 
								   associationDetail->location,
								   associationDetail->base_location_suffix,
								   associationDetail->assoc_location_suffix,
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
		
		delete associationDetail;
	}
	associationSTPCancelInsert.clear();
}

int NRCIF::findIDForService(mysqlpp::Connection &conn, CIFRecordNRBS *s, CIFRecordNRHD *h, bool exact, bool removeDoesntRunOn, bool noDateTo) {
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
		query << "SELECT id FROM schedules_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND date_from = " << mysqlpp::quote << s->date_from << " AND stp_indicator = " << mysqlpp::quote <<  s->stp_indicator;
	}
	else {
		if(s->date_to != "" && !noDateTo) {
			query << "SELECT id FROM schedules_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND (" << mysqlpp::quote << s->date_from << " BETWEEN date_from AND date_to) AND (" << mysqlpp::quote << s->date_to << " BETWEEN date_from AND date_to) " <<  runs_on << " AND stp_indicator = " << mysqlpp::quote <<  s->stp_indicator; 
		}
		else {
			query << "SELECT id FROM schedules_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND (" << mysqlpp::quote << s->date_from << " BETWEEN date_from AND date_to) " << runs_on << " AND stp_indicator = " << mysqlpp::quote <<  s->stp_indicator;
		}
	}
	
	string queryString = query.str();
		
	if(mysqlpp::StoreQueryResult res = query.store()) {
		if(res.num_rows() > 1 && exact) {
			return -atoi(res[0]["id"].c_str());
		}
		else if(res.num_rows() > 0) {
			return atoi(res[0]["id"].c_str());
		}
		
		if(!noDateTo && !exact)
			return NRCIF::findIDForService(conn, s, h, exact, removeDoesntRunOn, true);
		else if(noDateTo && !removeDoesntRunOn && !exact) 
			return NRCIF::findIDForService(conn, s, h, exact, true, true);
		else {
			// uncomment this if you want some verbose output on errors
			//cout << endl << "Error query: " << queryString << endl;
			throw 1;
		}
	}
	
	throw 2;
}

int NRCIF::findIDForServiceStpCancel(mysqlpp::Connection &conn, CIFRecordNRBS *s) {
	mysqlpp::Query query = conn.query();
		
	// find this service
	query << "SELECT train_uid FROM schedules_stpcancel_core_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND cancel_from = " << mysqlpp::quote << s->date_from;
	
	string queryString = query.str();
		
	if(mysqlpp::StoreQueryResult res = query.store()) {
		if(res.num_rows() > 1) {
			return -1;
		}
		else if(res.num_rows() > 0) {
			return 1;
		}
		
		// uncomment this if you want some verbose output on errors
		//cout << endl << "Error query: " << queryString << endl;
		throw 1;
	}
	
	throw 2;
}



void NRCIF::deleteService(mysqlpp::Connection &conn, int id) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM locations_t WHERE id = " << id;
	query.execute();
	
	query << "DELETE FROM locations_change_t WHERE id = " << id;
	query.execute();
	
	query << "DELETE FROM schedules_t WHERE id = " << id;
	query.execute();
}

void NRCIF::deleteSTPServiceCancellation(mysqlpp::Connection &conn, CIFRecordNRBS *s) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM schedules_stpcancel_core_t WHERE train_uid = " << mysqlpp::quote << s->uid << " AND cancel_from = " << mysqlpp::quote << s->date_from;
	query.execute();
}

int NRCIF::findIDForAssociation(mysqlpp::Connection &conn, CIFRecordNRAA *a, CIFRecordNRHD *h, bool exact, bool removeDoesntRunOn, bool noDateTo) {
	mysqlpp::Query query = conn.query(); 
	
	// create string to check assoc on dates...
	string assoc_on = "";
	if(removeDoesntRunOn) {
		if(a->assoc_mo != "" && a->assoc_mo != "0") assoc_on += " AND runs_mo = '" + a->assoc_mo + "'";
		if(a->assoc_tu != "" && a->assoc_tu != "0") assoc_on += " AND runs_tu = '" + a->assoc_tu + "'";
		if(a->assoc_we != "" && a->assoc_we != "0") assoc_on += " AND runs_we = '" + a->assoc_we + "'";
		if(a->assoc_th != "" && a->assoc_th != "0") assoc_on += " AND runs_th = '" + a->assoc_th + "'";
		if(a->assoc_fr != "" && a->assoc_fr != "0") assoc_on += " AND runs_fr = '" + a->assoc_fr + "'";
		if(a->assoc_sa != "" && a->assoc_sa != "0") assoc_on += " AND runs_sa = '" + a->assoc_sa + "'";
		if(a->assoc_su != "" && a->assoc_su != "0") assoc_on += " AND runs_su = '" + a->assoc_su + "'";
	}
	else {
		if(a->assoc_mo != "") assoc_on += " AND runs_mo = '" + a->assoc_mo + "'";
		if(a->assoc_tu != "") assoc_on += " AND runs_tu = '" + a->assoc_tu + "'";
		if(a->assoc_we != "") assoc_on += " AND runs_we = '" + a->assoc_we + "'";
		if(a->assoc_th != "") assoc_on += " AND runs_th = '" + a->assoc_th + "'";
		if(a->assoc_fr != "") assoc_on += " AND runs_fr = '" + a->assoc_fr + "'";
		if(a->assoc_sa != "") assoc_on += " AND runs_sa = '" + a->assoc_sa + "'";
		if(a->assoc_su != "") assoc_on += " AND runs_su = '" + a->assoc_su + "'";
	}
	
	// find this association
	if(exact) {		
		if(a->transaction_type != "D") {
			query << "SELECT id FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND date_from = " << mysqlpp::quote << a->date_from << " AND base_location_suffix = " << mysqlpp::quote << a->base_location_suffix << " AND assoc_location_suffix = " << mysqlpp::quote << a->assoc_location_suffix << " AND stp_indicator = " << mysqlpp::quote <<  a->stp_indicator;
		}
		else {
			query << "SELECT id FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND date_from = " << mysqlpp::quote << a->date_from << " AND stp_indicator = " << mysqlpp::quote <<  a->stp_indicator;
		}
	}
	else{
		if(a->transaction_type != "D") {
			if(a->date_to != "" && !noDateTo) {
				query << "SELECT id FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND (" << mysqlpp::quote << a->date_from << " BETWEEN date_from AND date_to) AND (" << mysqlpp::quote << a->date_to << " BETWEEN date_from AND date_to) AND base_location_suffix = " << mysqlpp::quote << a->base_location_suffix << " AND assoc_location_suffix = " << mysqlpp::quote << a->assoc_location_suffix << " " << assoc_on << " AND stp_indicator = " << mysqlpp::quote <<  a->stp_indicator; 
			}
			else {
				query << "SELECT id FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND (" << mysqlpp::quote << a->date_from << " BETWEEN date_from AND date_to) AND base_location_suffix = " << mysqlpp::quote << a->base_location_suffix << " AND assoc_location_suffix = " << mysqlpp::quote << a->assoc_location_suffix << "  " << assoc_on << " AND stp_indicator = " << mysqlpp::quote <<  a->stp_indicator;
			}
		}
		else {
			if(a->date_to != "" && !noDateTo) {
				query << "SELECT id FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND (" << mysqlpp::quote << a->date_from << " BETWEEN date_from AND date_to) AND (" << mysqlpp::quote << a->date_to << " BETWEEN date_from AND date_to) " << assoc_on << " AND stp_indicator = " << mysqlpp::quote <<  a->stp_indicator; 
			}
			else {
				query << "SELECT id FROM associations_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND (" << mysqlpp::quote << a->date_from << " BETWEEN date_from AND date_to) " << assoc_on << " AND stp_indicator = " << mysqlpp::quote <<  a->stp_indicator;
			}
		}
	}
			
	if(mysqlpp::StoreQueryResult res = query.store()) {		
		if(res.num_rows() > 1 && exact) {
			return -atoi(res[0]["id"].c_str());
		}
		else if(res.num_rows() > 0) {
			return atoi(res[0]["id"].c_str());
		}
		
		if(!noDateTo && !exact)
			return NRCIF::findIDForAssociation(conn, a, h, exact, removeDoesntRunOn, true);
		else if(noDateTo && !removeDoesntRunOn && !exact) 
			return NRCIF::findIDForAssociation(conn, a, h, exact, true, true);
		else {
			// uncomment this if you want some verbose output on errors
			//cout << endl << "Error query: " << queryString << endl;
			throw 1;
		}
	}
	
	throw 2;
}

int NRCIF::findIDForAssociationStpCancel(mysqlpp::Connection &conn, CIFRecordNRAA *a) {
	mysqlpp::Query query = conn.query();
		
	// find this service
	query << "SELECT main_train_uid FROM associations_stpcancel_core_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND cancel_from = " << mysqlpp::quote << a->date_from;
	
	string queryString = query.str();
		
	if(mysqlpp::StoreQueryResult res = query.store()) {
		if(res.num_rows() > 1) {
			return -1;
		}
		else if(res.num_rows() > 0) {
			return 1;
		}
		
		// uncomment this if you want some verbose output on errors
		//cout << endl << "Error query: " << queryString << endl;
		throw 1;
	}
	
	throw 2;
}

void NRCIF::deleteAssociation(mysqlpp::Connection &conn, int id) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM associations_t WHERE id = " << id;
	query.execute();
}


void NRCIF::deleteSTPAssociationCancellation(mysqlpp::Connection &conn, CIFRecordNRAA *a) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM associations_stpcancel_core_t WHERE main_train_uid = " << mysqlpp::quote << a->main_train_uid << " AND assoc_train_uid = " << mysqlpp::quote << a->assoc_train_uid << " AND location = " << mysqlpp::quote << a->location << " AND cancel_from = " << mysqlpp::quote << a->date_from;
	
	query.execute();
}
