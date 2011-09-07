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
#include "CIF.h"

// NR CIF
#include "NR-CIF/CIFRecordNRHD.cpp"
#include "NR-CIF/CIFRecordNRTITA.cpp"
#include "NR-CIF/CIFRecordNRTD.cpp"
#include "NR-CIF/CIFRecordNRBS.cpp"
#include "NR-CIF/CIFRecordNRLOLILT.cpp"

// database related stuff
#include "sqlNetRail.h"
//#include "databaseConfig.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <boost/progress.hpp>
#include <mysql++.h>

using namespace std;
using namespace boost;

#define TRUNCATE_ON_NEW true

void CIF::processCIFFile(const char* filePath) {
		
	mysqlpp::Connection conn;
	if(!conn.connect(sqlDatabase, sqlServer, sqlUsername, sqlPassword)) {
		cerr << "DB connection failed:" << conn.error() << endl;
		return;
	}

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
	long displayKb = fileSize / 1024;
	long displayMb = displayKb / 1024;
	
	if(file.is_open()) {
		string line;
		
		getline(file, line);
		if(line.substr(0, 2) == "HD") {
			/**** COMMON INTERFACE FILE (NETWORK RAIL) ORDER ************************\
			|  Surrounded in [] not implemented. If () not always there              |
			|                                                                        |
			|  HD Header                                                             |
			|  Tiploc:                                                               |
			|    TI - TIPLOC Insert                                                  |
			|    TA - TIPLOC Amend                                                   |
			|    TD - TIPLOC Delete                                                  |
			| [ AA Associations (order Delete, New, Replace) ]                       |
			|  Schedule:                                                             |
			|    BS - Basic Schedule                                                 |
			|    BX - Basic Schedule, additional information                         |
			|  [ TN - Train specific note record ]                                   |
			|    LO - Origin location                                                |
			|  [ CR - Change en route (always associated with following LI) ]        |
			|    LI - Intermediate location                                          |
			|    LT - Terminating location                                           |
			|  [ LN - Location note - may follow any LO, LI or LT ]                  |
			|  ZZ - trailing record                                                  |
			\************************************************************************/
			
			// Network Rail CIF subroutine
			CIFRecord *record;
			
			// we know that this will be the header, and in the implementation there is a try catch for mainframe id
			try {
				record = CIF::processNRCIFLine(line);
			}
			catch(int i) {
				if(i == -1) { 
					cerr << "The header file does not represent a valid CIF." << endl;
				}
				else {
					cerr << "Unknown error occured. Exiting..." << endl;
				}
				
				conn.disconnect();
				return;
			}
			
			CIFRecordNRHD *header; 
			
			if(record->getRecordType() == 00) {
				header = (CIFRecordNRHD *)record;
			}
			else {
				delete record;
				cerr << "Unable to process " << filePath << " as it is not a valid CIF" << endl;
				conn.disconnect();
				return;
			}
			
			cout << "===========================================================" << endl;
			cout << "Performing run on Network Rail CIF: " << filePath << endl;
			cout << "Importing " << header->curr_file_ref << " created for " << header->mainframe_user << endl;
			cout << "Generated on " << header->date_extract << " at " << header->time_extract << endl;
			cout << "Data from " << header->extract_start << " to " << header->extract_end << endl;
			cout << "===========================================================" << endl;
			
			mysqlpp::Query query = conn.query();
			
			if(header->update_type == "F") { // full update so am disabling the keys... as its so much quicker!				
				/*if((bool)TRUNCATE_ON_NEW == true) {
					cout << "INFO: Due to application settings, truncating the tables now due to full import." << endl;
					query.execute("TRUNCATE TABLE schedules");
					query.execute("TRUNCATE TABLE locations");
					query.execute("TRUNCATE TABLE tiplocs");
					
					cout << "INFO: Tables truncated. Continuing..." << endl;
				} */
				
				query.execute("ALTER TABLE schedules DISABLE KEYS");
				query.execute("ALTER TABLE locations DISABLE KEYS");
				query.execute("ALTER TABLE tiplocs DISABLE KEYS");
				cout << "INFO: Full update type - disabled keys on tables. Re-enabled once complete." << endl;
			}
			else {
				cout << "INFO: Modify update type - keys remaining on tables." << endl;
			}
			
			vector<tiplocs> tiplocInsert;
			vector<string> tiplocDelete;
			bool tiplocComplete = false;
			
			while(getline(file, line)) {try{
				record = CIF::processNRCIFLine(line);
				unsigned recordType = record->getRecordType();
				
				if(recordType == 1 || recordType == 2 || recordType == 3) { // tiploc insert or amend
					CIFRecordNRTITA *tiplocRecord = (CIFRecordNRTITA *)record;
					
					if(recordType == 2 || recordType == 3) {
						if(header->update_type == "F") {
							cerr << endl << "ERROR: TIPLOC AMEND record appeared in a full update. Exiting." << endl;
							delete record;
							conn.disconnect();
							return;
						}
						
						if(recordType == 2) {
							tiplocDelete.push_back(tiplocRecord->tiploc_code);
						}
						else if(recordType == 3) {
							tiplocDelete.push_back(tiplocRecord->old_tiploc);
						}
					}
					
					tiplocInsert.push_back(tiplocs(tiplocRecord->tiploc_code, tiplocRecord->nlc, tiplocRecord->tps_desc, tiplocRecord->stanox, tiplocRecord->crs, tiplocRecord->capri_desc));
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
						return;
					}
				}
				else if(recordType == 5) { // associations
					
				}
				else if(recordType == 6) { // insert
					// just finish any tiploc stuff...
					if(!tiplocComplete) {
						// delete the old tiplocs
						vector<string>::iterator dit;
						for(dit = tiplocDelete.begin(); dit < tiplocDelete.end(); dit++) {
							query << "DELETE FROM tiplocs WHERE tiploc = " << mysqlpp::quote << *dit;
							query.execute();
						}
						tiplocDelete.clear();
			
						// insert the new tiplocs
						vector<tiplocs>::iterator iit;
						for(iit = tiplocInsert.begin(); iit < tiplocInsert.end(); iit++) {
							try {
								query.insert(*iit);
								query.execute();
							}
							catch(mysqlpp::BadQuery& e) {
								cout << "INFO: Inserting location error: " << e.what() << endl;
							}
						}
						tiplocInsert.clear();
						
						tiplocComplete = true;
					}
				
					CIFRecordNRBS *scheduleDetail = (CIFRecordNRBS *)record;
					
					if(scheduleDetail->transaction_type == "R" || scheduleDetail->transaction_type == "D") {
						if(header->update_type == "F") {
							cerr << endl << "ERROR: SCHEDULE REVISE/DELETE record appeared in a full update. Exiting." << endl;
							delete record;
							conn.disconnect();
							return;
						}
						
						if(scheduleDetail->stp_indicator != "C") { // deletion of a non STP cancel.
							string uuid;
						
							try {
								uuid = CIF::findNRCIFService(conn, scheduleDetail->uid, scheduleDetail->date_from);
							}
							catch(int e) {
								cerr << "ERROR: Unable to locate service to delete/amend. Exiting." << endl;
								delete record;
								conn.disconnect();
								return;
							}
						
							CIF::deleteNRCIFService(conn, uuid);
							uuid.clear();
						}
						else if(scheduleDetail->stp_indicator == "C") {
							// deletion of an STP cancel...
							string uuid;
							
							// find the permament service relating to these dates
							try {
								uuid = CIF::findNRCIFPermServiceBtwnDates(conn, scheduleDetail->uid, scheduleDetail->date_from, schedule->date_to);
							}
							catch(int e) {
								cerr << "ERROR: Unable to locate service to remove STP cancel" << endl;
								delete record;
								conn.disconnect();
								return;
							}
							
							CIF::deleteNRCIFSTPCancel(conn, uuid, scheduleDetail->date_from, scheduleDetail->date_to);
							uuid.clear();
						}
					}
										
					if(scheduleDetail->transaction_type == "N" || scheduleDetail->transaction_type == "R") {
						if(scheduleDetail->stp_indicator != "C") { 
						
							// the BX record is always the next line, so just parse this straight back to the scheduleDetail.
							getline(file, line);
							scheduleDetail->mergeWithBX(line);
						
						// ok set it up and now get schedule locations
							vector<locations> locs;
							
							CIFRecord *schedulerec;
							int location_order = 0;
							
							while(1) {
								getline(file, line);
								schedulerec = CIF::processNRCIFLine(line);
								
								if(schedulerec->getRecordType() == 7) {
									CIFRecordNRLOLILT *location = (CIFRecordNRLOLILT *)schedulerec;
									
									locs.push_back(locations(scheduleDetail->unique_id, location_order, location->record_type, location->tiploc, location->tiploc_instance, location->arrival, location->public_arrival, location->pass, location->departure, location->public_departure, location->platform, location->line, location->path, location->engineering_allowance, location->pathing_allowance, location->performance_allowance, location->activity));
								
									if(location->record_type == "LT") { delete schedulerec; break; }
									
									location_order++;
								}
								
								delete schedulerec;
							}
							
							try {
								mysqlpp::Query query = conn.query();
								
								// load up the schedule
								schedules row(scheduleDetail->unique_id,
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
											  scheduleDetail->data_source);
							
								query.insert(row);
								query.execute(); 
							
								// insert policy for remaining stuff
								mysqlpp::Query::RowCountInsertPolicy<> insert_policy(1000);
								query.insertfrom(locs.begin(), locs.end(), insert_policy);
								locs.clear();
							}
							catch(mysqlpp::BadQuery& e) {
								cerr << "Error (query): " << e.what() << endl;
								conn.disconnect();
								return;
							}
						}
						else if(scheduleDetail->stp_indicator == "C") {
							// this is a STP CAN of LTP schedule
							// locate the uuid of the service being cancelled...
							
							// get uuid of service being STP cancelled
							string uuid;
							try {
								uuid = CIF::findNRCIFPermServiceBtwnDates(conn, scheduleDetail->uid, scheduleDetail->date_from, schedule->date_to);
							}
							catch(int e) {
								cerr << "ERROR: Unable to locate service to STP cancel" << endl;
								delete record;
								conn.disconnect();
								return;
							}
							
							mysqlpp::Query query = conn.query();
							
							schedules_stpcancel row(uuid, mysqlpp::sql_date(scheduleDetail->date_from), mysqlpp::sql_date(scheduleDetail->date_to));
							
							query.insert(row);
							query.execute();
							uuid.clear();
						} 
					}
				}
				
			}catch(int e){ continue; } delete record; }			
			
			if(header->update_type == "F") { 
				cout << "INFO: Full update - re-enabling keys... may take a few minutes." << endl;
				query.execute("ALTER TABLE schedules ENABLE KEYS");
				query.execute("ALTER TABLE locations ENABLE KEYS");
				query.execute("ALTER TABLE tiplocs ENABLE KEYS");
				cout << "INFO: Keys updated. Completed " << header->curr_file_ref << ". " << endl;
			}
			else {
				cout << "INFO: Complete. " << endl;
			}
			
			delete header;
		}
		else {
			// Not valid file subroutine
		}

	}
	
	conn.disconnect();
	file.close();	
}

CIFRecord* CIF::processNRCIFLine(string record) {
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
	else {
		throw 1;
	}
}

string CIF::findNRCIFService(mysqlpp::Connection &conn, string uniqueId, string startDate, string stpIndicator) {
	mysqlpp::Query query = conn.query();
	
	if(stpIndicator == "") {
		query << "SELECT uuid FROM schedules WHERE train_uid = " << mysqlpp::quote << uniqueId << " AND date_from = " << mysqlpp::quote << startDate << " AND stpIndicator = 'N' LIMIT 0, 1";
	}
	else {
		query << "SELECT uuid FROM schedules WHERE train_uid = " << mysqlpp::quote << uniqueId << " AND date_from = " << mysqlpp::quote << startDate << " AND stpIndicator = " << mysqlpp::quote << stpIndicator << " LIMIT 0, 1";
	}
	
	if(mysqlpp::StoreQueryResult res = query.store()) {
		if(res.num_rows() > 0) {
			return res[0]["uuid"].c_str();
		}
		
		throw 1;
	}
	
	throw 2;
}

string CIF::findNRCIFPermServiceBtwnDates(mysqlpp::Connection &conn, string uniqueId, string startDate, string endDate) {
	mysqlpp::Query query = conn.query();
	
	query << "SELECT uuid FROM schedules WHERE train_uid = " << mysqlpp::quote << uniqueId << " AND ((" << mysqlpp::quote << startDate << " BETWEEN date_from AND date_to) AND (" << mysqlpp::quote << endDate << " BETWEEN date_from AND date_to)) AND stp_indicator = 'P' LIMIT 0,1";
	
	if(mysqlpp::StoreQueryResult res = query.store()) {
		if(res.num_rows() > 0) {
			return res[0]["uuid"].c_str();
		}
		
		throw 1;
	}
	
	throw 2;
}

void CIF::deleteNRCIFService(mysqlpp::Connection &conn, string uuid) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM locations WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
	
	query << "DELETE FROM schedules WHERE uuid = " << mysqlpp::quote << uuid;
	query.execute();
}

void CIF::deleteNRCIFSTPCancel(mysqlpp::Connection &conn, string uuid, string cancelFrom, string cancelTo) {
	mysqlpp::Query query = conn.query();
	
	query << "DELETE FROM schedules_stpcancel WHERE uuid = " << mysqlpp::quote << uuid << " AND cancel_from = " << mysqlpp::quote << cancelFrom << " AND cancel_to = " << mysqlpp::quote << cancelTo;
	query.execute();
}