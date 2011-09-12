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

#include "classes/NRCIF.cpp"
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
	query.exec("DROP TABLE IF EXISTS associations_old, associations_stpcancel_old, locations_old, locations_change_old, schedules_old, schedules_stpcancel_old, tiplocs_old, associations_t, associations_stpcancel_t, locations_t, locations_change_t, schedules_t, schedules_stpcancel_t, tiplocs_t");
	
	// ok, we are now going to begin the actual processing by creating temporary tables
	cout << "INFO: Creating temporary tables..." << endl;
	
	if(truncateTables) {
		query.exec("CREATE TABLE associations_t LIKE associations");
		query.exec("CREATE TABLE associations_stpcancel_t LIKE associations_stpcancel");
		query.exec("CREATE TABLE locations_t LIKE locations");
		
		if(disableKeys) {
			query.exec("ALTER TABLE locations_t DROP INDEX `tiploc_code`, DROP INDEX `arrival`,DROP INDEX `public_arrival`,DROP INDEX `pass`, DROP INDEX `departure`,DROP INDEX `public_departure`, DROP INDEX `uuid`");
		}
		else {
			query.exec("ALTER TABLE locations_t DROP INDEX `tiploc_code`, DROP INDEX `arrival`, DROP INDEX `public_arrival`, DROP INDEX `pass`, DROP INDEX `departure`, DROP INDEX `public_departure`");
		}
		
		query.exec("CREATE TABLE locations_change_t LIKE locations_change");
		query.exec("CREATE TABLE schedules_t LIKE schedules");
		query.exec("CREATE TABLE schedules_stpcancel_t LIKE schedules_stpcancel");
		query.exec("CREATE TABLE tiplocs_t LIKE tiplocs");
	}
	else {
		cout << "INFO: Inserting associations into temporary table..." << endl;
		query.exec("CREATE TABLE associations_t LIKE associations");
		query.exec("INSERT INTO associations_t SELECT * FROM associations");
		
		cout << "INFO: Inserting associations_stpcancel into temporary table..." << endl;
		query.exec("CREATE TABLE associations_stpcancel_t LIKE associations_stpcancel");
		query.exec("INSERT INTO associations_stpcancel_t SELECT * FROM associations_stpcancel");
		
		cout << "INFO: Inserting locations into temporary table..." << endl;
		query.exec("CREATE TABLE locations_t SELECT * FROM locations"); // temp locations
		
		if(!disableKeys) {
			cout << "INFO: Generating UUID key on locations_t" << endl;
			query.exec("ALTER TABLE locations_t ADD INDEX (`uuid`)");
		}
		
		cout << "INFO: Inserting locations_change into temporary table..." << endl;
		query.exec("CREATE TABLE locations_change_t LIKE locations_change");
		query.exec("INSERT INTO locations_change_t SELECT * FROM locations_change");
		
		cout << "INFO: Inserting schedules into temporary table..." << endl;
		query.exec("CREATE TABLE schedules_t LIKE schedules");
		query.exec("INSERT INTO schedules_t SELECT * FROM schedules");
		
		cout << "INFO: Inserting schedules_stpcancel into temporary table..." << endl;
		query.exec("CREATE TABLE schedules_stpcancel_t LIKE schedules_stpcancel");
		query.exec("INSERT INTO schedules_stpcancel_t SELECT * FROM schedules_stpcancel");
		
		cout << "INFO: Inserting tiplocs into temporary table..." << endl;
		query.exec("CREATE TABLE tiplocs_t LIKE tiplocs");
		query.exec("INSERT INTO tiplocs_t SELECT * FROM tiplocs");
	}
	
	cout << "INFO: Temporary tables created..." << endl;
	
	if(disableKeys) {
		if(disableImportantKeys) {
			query.exec("ALTER TABLE associations_t DISABLE KEYS;");
		}
		
		query.exec("ALTER TABLE associations_stpcancel_t DISABLE KEYS;");
		// locations won't have its keys disabled here as there won't be any!
		query.exec("ALTER TABLE locations_change_t DISABLE KEYS;");
		
		if(disableImportantKeys) {
			query.exec("ALTER TABLE schedules_t DISABLE KEYS;");
		}
		else {
			query.exec("ALTER TABLE schedules_t DROP INDEX `train_identity`, DROP INDEX `bank_hol`, DROP INDEX `status`, DROP INDEX `atoc_code`"); 
		}
		
		query.exec("ALTER TABLE schedules_stpcancel_t DISABLE KEYS;");
		query.exec("ALTER TABLE tiplocs_t DISABLE KEYS;");
		
		cout << "INFO: Keys have been disabled on tables, they will be re-enabled at the end." << endl;
	}	
	
	// at this point, all tables have their keys as per normal EXCEPT locations which 
	//   if disableKeys = TRUE, then has no keys
	//   else                   has a uuid key
	
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
					filePaths.insert(filePath + itr->leaf());
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
		
		cout << "INFO: Enabling keys on associations_stpcancel..." << endl;
		query.exec("ALTER TABLE associations_stpcancel_t ENABLE KEYS;");
		
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
		
		cout << "INFO: Enabling keys on schedules_stpcancel..." << endl;
		query.exec("ALTER TABLE schedules_stpcancel_t ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on tiplocs..." << endl;
		query.exec("ALTER TABLE tiplocs_t ENABLE KEYS;");
		
		cout << "INFO: Complete, all keys re-enabled and recalculated." << endl;
	}
	
	cout << "INFO: Adding keys on locations (temp table) - may take a while..." << endl;
	if(disableKeys) {
		query.exec("ALTER TABLE locations_t ADD INDEX (`uuid`), ADD INDEX (`tiploc_code`), ADD INDEX (`arrival`), ADD INDEX  (`public_arrival`), ADD INDEX (`pass`), ADD INDEX (`departure`), ADD INDEX (`public_departure`)");
	}
	else {
		query.exec("ALTER TABLE locations_t ADD INDEX (`tiploc_code`), ADD INDEX (`arrival`), ADD INDEX (`public_arrival`), ADD INDEX (`pass`), ADD INDEX (`departure`), ADD INDEX (`public_departure`)");
	}
	
	cout << "INFO: Completed adding keys on locations" << endl;
	
	cout << "INFO: Now moving current tables out of the way..." << endl;
	query.exec("RENAME TABLE associations TO associations_old, associations_t TO associations, associations_stpcancel TO associations_stpcancel_old, associations_stpcancel_t TO associations_stpcancel, locations_change TO locations_change_old, locations_change_t TO locations_change, schedules TO schedules_old, schedules_t TO schedules, schedules_stpcancel TO schedules_stpcancel_old, schedules_stpcancel_t TO schedules_stpcancel, tiplocs TO tiplocs_old, tiplocs_t TO tiplocs, locations TO locations_old, locations_t TO locations");
	cout << "INFO: Tables out of the way, finished!" << endl;
	
	conn.disconnect();
	
	return 0;
}
