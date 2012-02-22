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

#ifndef _NRCIF_INC_
	#define _NRCIF_INC_
	#include "classes/NRCIF.h"
#endif

#include "databaseConfig.h"
#include <mysql++.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/timer.hpp>

using namespace std;

int main(int argc, char *argv[]) {	
	cout << "CIF Reader - Copyright 2011 Tom Cairns" << endl << 	
			"This program comes with ABSOLUTELY NO WARRANTY. This is free software and you are " << endl <<
			"welcome to redistribute it under certain conditions, see LICENCE." << endl << endl;
	
	bool truncateTables = false;
	bool networkRailCIF = true;
	bool disableKeys = false;
	bool disableImportantKeys = true;
	vector<string> files;
	vector<string> directories;
	int fileStatus;
	struct stat st_buf;
	
	// process the arguments
	string arg;
	for(int i = 1; i < argc; i++) {
		arg = argv[i];

		if(arg == "-t" || arg == "--truncate-tables") {
			truncateTables = true;
		}
		else if(arg == "-a" || arg == "--atco") {
			networkRailCIF = false;
		}
		else if(arg == "-k" || arg == "--disable-keys") {
			disableKeys = true;
		}
		else if(arg == "-d" || arg == "--do-not-disable-important-keys") {
			disableImportantKeys = false;
		}
		else if(arg == "-h" || arg == "--help") {
			cout << "Commands:\n\n\t-t or --truncate-tables\n\t\tEmpties all the tables to allow clean import.\n\n\t-a or --atco\n\t\tEnables ATCO-CIF import mode\n\n\t-k or --disable-keys\n\t\tDisables ALL the keys on the tables to allow a much faster import. Re-enables and re-calculates keys after program runs.\n\n\t-d or --do-not-disable-important-keys\n\t\tRequires -k, except does not disable keys on schedules (best used if on overlay)" << endl;
			return 0;
		}
		else {
			fileStatus = stat(argv[i], &st_buf);
			if(fileStatus != 0) { 
				arg.clear();
				continue;
			}
			
			if(S_ISREG(st_buf.st_mode)) { 
				files.push_back(arg); 
			}
			else if(S_ISDIR(st_buf.st_mode)) { 
				directories.push_back(arg);
			}
		}
		arg.clear();
	}
	
	if(files.size() == 0 && directories.size() == 0) {
		cerr << "No files or directories to process given. " << endl; 
		return 1;
	}
	
	// attempt mysql connection
	mysqlpp::Connection conn;
	if(!conn.connect(sqlDatabase, sqlServer, sqlUsername, sqlPassword)) {
		cerr << "DB connection failed:" << conn.error() << endl;
		return 1;
	}
	
	// ok, so we know that whatever it is exists... users own fault if they
	// fail on this in my view...! so let's go for it and deal with truncate
	// tables and disable keys
	
	mysqlpp::Query query = conn.query();
	
	// drop any old tables that might exist
	query.exec("DROP TABLE IF EXISTS associations_old, associations_stpcancel_old, associations_stpcancel_core_old, locations_old, locations_change_old, schedules_old, schedules_stpcancel_old, schedules_stpcancel_core_old, tiplocs_old, schedules_cache_old, tiplocs_cache_old, associations_t, associations_stpcancel_t, associations_stpcancel_core_t, locations_t, locations_change_t, schedules_t, schedules_stpcancel_t, schedules_stpcancel_core_t, tiplocs_t, schedules_cache_t, tiplocs_cache_t");
	
	// ok, we are now going to begin the actual processing by creating temporary tables
	cout << "INFO: Creating temporary tables..." << endl;
	
	if(truncateTables) {
		query.exec("CREATE TABLE associations_t LIKE associations");
		query.exec("CREATE TABLE associations_stpcancel_t LIKE associations_stpcancel");
		query.exec("CREATE TABLE associations_stpcancel_core_t LIKE associations_stpcancel_core");
		query.exec("CREATE TABLE locations_t LIKE locations");
		
		if(disableKeys) {
			query.exec("ALTER TABLE locations_t DROP INDEX `location_type`, DROP INDEX `tiploc_code`, DROP INDEX `order_time`,DROP INDEX `public_call`,DROP INDEX `actual_call`, DROP INDEX `id`");
		}
		else {
			query.exec("ALTER TABLE locations_t DROP INDEX `location_type`, DROP INDEX `tiploc_code`, DROP INDEX `order_time`,DROP INDEX `public_call`,DROP INDEX `actual_call`");
		}
		
		query.exec("CREATE TABLE locations_change_t LIKE locations_change");
		query.exec("CREATE TABLE schedules_t LIKE schedules");
		query.exec("CREATE TABLE schedules_stpcancel_t LIKE schedules_stpcancel");
		query.exec("CREATE TABLE schedules_stpcancel_core_t LIKE schedules_stpcancel_core");
		query.exec("CREATE TABLE tiplocs_t LIKE tiplocs");
		query.exec("CREATE TABLE tiplocs_cache_t LIKE tiplocs_cache");
		query.exec("CREATE TABLE schedules_cache_t LIKE schedules_cache");
	}
	else {
		cout << "INFO: Inserting associations into temporary table..." << endl;
		query.exec("CREATE TABLE associations_t LIKE associations");
		query.exec("INSERT INTO associations_t SELECT * FROM associations");
		
		cout << "INFO: Inserting associations_stpcancel into temporary table..." << endl;
		query.exec("CREATE TABLE associations_stpcancel_core_t LIKE associations_stpcancel_core");
		query.exec("INSERT INTO associations_stpcancel_core_t SELECT * FROM associations_stpcancel_core");
		
		cout << "INFO: Inserting locations into temporary table..." << endl;
		query.exec("CREATE TABLE locations_t SELECT * FROM locations"); // temp locations
		
		if(!disableKeys) {
			cout << "INFO: Generating ID key on locations_t" << endl;
			query.exec("ALTER TABLE locations_t ADD INDEX (`id`)");
		}
		
		cout << "INFO: Inserting locations_change into temporary table..." << endl;
		query.exec("CREATE TABLE locations_change_t LIKE locations_change");
		query.exec("INSERT INTO locations_change_t SELECT * FROM locations_change");
		
		cout << "INFO: Inserting schedules into temporary table..." << endl;
		query.exec("CREATE TABLE schedules_t LIKE schedules");
		query.exec("INSERT INTO schedules_t SELECT * FROM schedules");
		
		cout << "INFO: Inserting schedules_stpcancel_core into temporary table..." << endl;
		query.exec("CREATE TABLE schedules_stpcancel_core_t LIKE schedules_stpcancel_core");
		query.exec("INSERT INTO schedules_stpcancel_core_t SELECT * FROM schedules_stpcancel_core");
		
		cout << "INFO: Inserting tiplocs into temporary table..." << endl;
		query.exec("CREATE TABLE tiplocs_t LIKE tiplocs");
		query.exec("INSERT INTO tiplocs_t SELECT * FROM tiplocs");
		
		// create cache temp table
		query.exec("CREATE TABLE schedules_cache_t LIKE schedules_cache");
		query.exec("CREATE TABLE tiplocs_cache_t LIKE tiplocs_cache");
		query.exec("CREATE TABLE schedules_stpcancel_t LIKE schedules_stpcancel");
		query.exec("CREATE TABLE associations_stpcancel_t LIKE associations_stpcancel");
	}
	
	cout << "INFO: Temporary tables created..." << endl;
	
	if(disableKeys) {
		if(disableImportantKeys) {
			query.exec("ALTER TABLE associations_t DISABLE KEYS;");
		}
		
		query.exec("ALTER TABLE associations_stpcancel_core_t DISABLE KEYS;");
		// locations won't have its keys disabled here as there won't be any!
		query.exec("ALTER TABLE locations_change_t DISABLE KEYS;");
		
		if(disableImportantKeys) {
			query.exec("ALTER TABLE schedules_t DISABLE KEYS;");
		}
		else {
			query.exec("ALTER TABLE schedules_t DROP INDEX `train_identity`, DROP INDEX `bank_hol`, DROP INDEX `status`, DROP INDEX `atoc_code`"); 
		}
		
		query.exec("ALTER TABLE schedules_stpcancel_core_t DISABLE KEYS;");
		query.exec("ALTER TABLE tiplocs_t DISABLE KEYS;");
		
		cout << "INFO: Keys have been disabled on tables, they will be re-enabled at the end." << endl;
	}	
	
	// at this point, all tables have their keys as per normal EXCEPT locations which 
	//   if disableKeys = TRUE, then has no keys
	//   else                   has a id key
	
	bool operationFailed = false;
	
	for(vector<string>::iterator fit = files.begin(); fit < files.end(); ++fit) {
		/* hack to get around weird seg fault */
		mysqlpp::Connection connection;
		if(!connection.connect(sqlDatabase, sqlServer, sqlUsername, sqlPassword)) {
			cerr << "DB connection failed:" << conn.error() << endl;
			return 1;
		}
		
		connection.set_option(new mysqlpp::MultiStatementsOption(true));
			
		if(networkRailCIF){
			if(!NRCIF::processFile(connection, (*fit).c_str())) {
				operationFailed = true;
				connection.disconnect();
				break;
			}
		}
		else {}
		
		connection.disconnect();
	}
	
	files.clear();
	
	if(!operationFailed) {
		for(vector<string>::iterator dit = directories.begin(); dit < directories.end(); ++dit) {
			boost::filesystem::directory_iterator end_itr;
			string filePath = *dit;
			if(filePath.substr(filePath.length() -1, 1) != "/") filePath += "/";
			
			set<string> filePaths;
			
			for(boost::filesystem::directory_iterator itr(*dit); itr != end_itr; ++itr) {
				if(boost::filesystem::is_directory(*itr)) continue;
				else if(boost::filesystem::exists(*itr)) {
					filePaths.insert(itr->path().string());
				}
			}
			
			for(set<string>::iterator it = filePaths.begin(); it != filePaths.end(); it++) {
				/* hack to get around weird seg fault */
				mysqlpp::Connection connection;
				if(!connection.connect(sqlDatabase, sqlServer, sqlUsername, sqlPassword)) {
					cerr << "DB connection failed:" << conn.error() << endl;
					return 1;
				}
				
				connection.set_option(new mysqlpp::MultiStatementsOption(true));
				
				if(networkRailCIF){
					if(!NRCIF::processFile(connection, (*it).c_str())) {
						operationFailed = true;
						connection.disconnect();
						break;
					}
				}
				
				connection.disconnect();
			}
			
			filePaths.clear();
		}
	}
	
	directories.clear();
	
	// check failure...
	if(operationFailed) {
		cout << "ERROR: Failure has taken place. Update will not be completed..." << endl;
		conn.disconnect();
		return 1;
	}
	
	if(disableKeys) {
		cout << "INFO: Now re-enabling keys, this may take up to around 5 minutes..." << endl;
		if(disableImportantKeys) {
			cout << "INFO: Enabling keys on associations..." << endl;
			query.exec("ALTER TABLE associations_t ENABLE KEYS;");
		}
		
		cout << "INFO: Enabling keys on associations_stpcancel_core..." << endl;
		query.exec("ALTER TABLE associations_stpcancel_core_t ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on locations_change..." << endl;
		query.exec("ALTER TABLE locations_change_t ENABLE KEYS;");
		
		if(disableImportantKeys) {
			cout << "INFO: Enabling keys on schedules..." << endl;
			query.exec("ALTER TABLE schedules_t ENABLE KEYS;");
		}
		else {
			cout << "INFO: Adding keys on schedules..." << endl;
			query.exec("ALTER TABLE schedules_t ADD INDEX (`train_identity`), ADD INDEX (`bank_hol`), ADD INDEX (`status`), ADD INDEX (`atoc_code`)"); 
		}
		
		cout << "INFO: Enabling keys on schedules_stpcancel_core..." << endl;
		query.exec("ALTER TABLE schedules_stpcancel_core_t ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on tiplocs..." << endl;
		query.exec("ALTER TABLE tiplocs_t ENABLE KEYS;");
		
		cout << "INFO: Complete, all keys re-enabled and recalculated." << endl;
	}
	
	cout << "INFO: Adding keys on locations (temp table) - may take a while..." << endl;
	if(disableKeys) {
		query.exec("ALTER TABLE locations_t ADD INDEX (`id`), ADD INDEX (`location_type`), ADD INDEX (`tiploc_code`), ADD INDEX (`order_time`), ADD INDEX  (`public_call`), ADD INDEX (`actual_call`)");
	}
	else {
		query.exec("ALTER TABLE locations_t ADD INDEX (`location_type`), ADD INDEX (`tiploc_code`), ADD INDEX (`order_time`), ADD INDEX  (`public_call`), ADD INDEX (`actual_call`)");
	}
	
	cout << "INFO: Completed adding keys on locations" << endl;
	
	
	cout << "INFO: Regenerating STP cancels..." << endl;
	query.exec("insert into schedules_stpcancel_t select s.id as id, ssc.cancel_from, ssc.cancel_to, ssc.cancel_mo, ssc.cancel_tu, ssc.cancel_we, ssc.cancel_th, ssc.cancel_fr, ssc.cancel_sa, ssc.cancel_su from schedules_stpcancel_core_t ssc left join schedules_t s on s.train_uid = ssc.train_uid and s.stp_indicator = 'P' and ssc.cancel_to >= s.date_from");
	query.exec("insert into associations_stpcancel_t select a.id as id, asc.cancel_from, asc.cancel_to, asc.cancel_mo, asc.cancel_tu, asc.cancel_we, asc.cancel_th, asc.cancel_fr, asc.cancel_sa, asc.cancel_su from associations_stpcancel_core_t as `asc` left join associations_t a on a.main_train_uid = asc.main_train_uid and a.assoc_train_uid = asc.assoc_train_uid and a.location = asc.location and a.stp_indicator = 'P' and asc.cancel_to >= a.date_from");
	cout << "INFO: Building schedule cache..." << endl;
	query.exec("INSERT INTO schedules_cache_t SELECT l1.id, l1.tiploc_code as origin, l1.departure as departure, l1.public_departure as public_origin, l2.tiploc_code as destination, l2.arrival as destination_time, l2.public_arrival as public_destination FROM locations_t l1 LEFT JOIN locations_t l2 ON l1.id = l2.id AND l2.location_type = 'LT' WHERE l1.location_type = 'LO'");
	cout << "INFO: Building TIPLOC cache..." << endl;
	query.exec("INSERT INTO tiplocs_cache_t SELECT additional_tiploc tiploc, t.nalco, t.tps_description, t.stanox, t.crs, t.description FROM locations_alternatives la LEFT JOIN tiplocs_t t ON la.tiploc = t.tiploc UNION SELECT t.* FROM tiplocs_t t LEFT JOIN locations_alternatives la ON la.additional_tiploc = t.tiploc WHERE la.tiploc IS NULL ORDER BY tiploc ASC");
	cout << "INFO: Cache building completed..." << endl;
	
	cout << "INFO: Now moving current tables out of the way..." << endl;
	query.exec("RENAME TABLE associations TO associations_old, associations_t TO associations, associations_stpcancel TO associations_stpcancel_old, associations_stpcancel_t TO associations_stpcancel, associations_stpcancel_core TO associations_stpcancel_core_old, associations_stpcancel_core_t TO associations_stpcancel_core, locations_change TO locations_change_old, locations_change_t TO locations_change, schedules TO schedules_old, schedules_t TO schedules, schedules_stpcancel TO schedules_stpcancel_old, schedules_stpcancel_t TO schedules_stpcancel, schedules_stpcancel_core TO schedules_stpcancel_core_old, schedules_stpcancel_core_t TO schedules_stpcancel_core, tiplocs TO tiplocs_old, tiplocs_t TO tiplocs, locations TO locations_old, locations_t TO locations, schedules_cache TO schedules_cache_old, schedules_cache_t TO schedules_cache, tiplocs_cache TO tiplocs_cache_old, tiplocs_cache_t TO tiplocs_cache");
	cout << "INFO: Tables out of the way, finished!" << endl;
	
	conn.disconnect();
	
	return 0;
}
