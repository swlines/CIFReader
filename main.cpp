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

using namespace std;

int main(int argc, char *argv[]) {	
	cout << "CIF Explorer - Copyright 2011 Tom Cairns" << endl << 	
			"This program comes with ABSOLUTELY NO WARRANTY. This is free software and you are " << endl <<
			"welcome to redistribute it under certain conditions, see LICENCE." << endl << endl;
	
	bool truncateTables = false;
	bool networkRailCIF = true;
	bool disableKeys = false;
	char *location = argv[(argc-1)];
	
	// process the arguments
	string arg;
	for(int i = 0; i < argc; i++) {
		arg = argv[i];

		if(arg == "-t" || arg == "--truncate-tables") {
			truncateTables = true;
		}
		if(arg == "-a" || arg == "--atco") {
			networkRailCIF = false;
		}
		if(arg == "-k" || arg == "--disable-keys") {
			disableKeys = true;
		}
		if(arg == "-h" || arg == "--help") {
			cout << "Commands:\n\n\t-t or --truncate-tables\n\t\tEmpties all the tables to allow clean import.\n\n\t-a or --atco\n\t\tEnables ATCO-CIF import mode\n\n\t-k or --disable-keys\n\t\tDisables the keys on the tables to allow a much faster import. Re-enables and re-calculates keys after program runs." << endl;
			return 0;
		}
		arg.clear();
	}
	
	// attempt mysql connection
	mysqlpp::Connection conn;
	if(!conn.connect(sqlDatabase, sqlServer, sqlUsername, sqlPassword)) {
		cerr << "DB connection failed:" << conn.error() << endl;
		return 1;
	}
	
	struct stat st_buf;
	int fileStatus;
	
	fileStatus = stat(location, &st_buf);
	if(fileStatus != 0) { 
		cout << "Error opening file/directory." << endl;
		conn.disconnect();
		return 1;
	}
	
	// ok, so we know that whatever it is exists... users own fault if they
	// fail on this in my view...! so let's go for it and deal with truncate
	// tables and disable keys
	
	mysqlpp::Query query = conn.query();
	
	if(truncateTables) {
		query.exec("TRUNCATE TABLE associations;");
		query.exec("TRUNCATE TABLE associations_stpcancel;");
		query.exec("TRUNCATE TABLE locations;");
		query.exec("TRUNCATE TABLE schedules;");
		query.exec("TRUNCATE TABLE schedules_stpcancel;");
		query.exec("TRUNCATE TABLE tiplocs;");
		
		cout << "INFO: Tables have been emptied." << endl;
	}
	
	if(disableKeys) {
		query.exec("ALTER TABLE associations DISABLE KEYS;");
		query.exec("ALTER TABLE associations_stpcancel DISABLE KEYS;");
		query.exec("ALTER TABLE locations DISABLE KEYS;");
		query.exec("ALTER TABLE schedules DISABLE KEYS;");
		query.exec("ALTER TABLE schedules_stpcancel DISABLE KEYS;");
		query.exec("ALTER TABLE tiplocs DISABLE KEYS;");
		
		cout << "INFO: Keys have been disabled on tables, they will be re-enabled at the end." << endl;
	}	
	
	if(S_ISREG(st_buf.st_mode)) {
		if(networkRailCIF){
			NRCIF::processFile(conn, location);
		}
		else {}
	}
	else if(S_ISDIR(st_buf.st_mode)) {
		
	
		boost::filesystem::directory_iterator end_itr;
		string filePath(location);
		if(filePath.substr(filePath.length() -1, 1) != "/") filePath += "/";
		
		set<string> filePaths;
		
		for(boost::filesystem::directory_iterator itr(location); itr != end_itr; ++itr) {
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
			
			if(networkRailCIF){
				NRCIF::processFile(connection, (*b).c_str());
			}
			
			connection.disconnect();
		}
		
		filePaths.clear();
	}
	
	if(disableKeys) {
		cout << "INFO: Now re-enabling keys, this may take up to around 5 minutes..." << endl;
		cout << "INFO: Enabling keys on associations..." << endl;
		query.exec("ALTER TABLE associations ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on associations_stpcancel..." << endl;
		query.exec("ALTER TABLE associations_stpcancel ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on locations..." << endl;
		query.exec("ALTER TABLE locations ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on schedules..." << endl;
		query.exec("ALTER TABLE schedules ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on schedules_stpcancel..." << endl;
		query.exec("ALTER TABLE schedules_stpcancel ENABLE KEYS;");
		
		cout << "INFO: Enabling keys on tiplocs..." << endl;
		query.exec("ALTER TABLE tiplocs ENABLE KEYS;");
		
		cout << "INFO: Complete, all keys re-enabled and recalculated." << endl;
	}
	
	conn.disconnect();
	
	return 0;
}
