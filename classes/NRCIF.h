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

#include <string>
#include <vector>
#include <mysql++.h>

#include "sqlNetRail.h"
#include "NR-CIF/CIFRecordNRHD.cpp"
#include "NR-CIF/CIFRecordNRTITA.cpp"
#include "NR-CIF/CIFRecordNRTD.cpp"
#include "NR-CIF/CIFRecordNRBS.cpp"
#include "NR-CIF/CIFRecordNRLOLILT.cpp"
#include "NR-CIF/CIFRecordNRAA.cpp"
#include "NR-CIF/CIFRecordNRCR.cpp"

using namespace std;

class NRCIF {
	public:
		static bool processFile(mysqlpp::Connection &conn, const char* filePath);
		
	private:
		static CIFRecord* processLine(string record);
		static void runTiploc(mysqlpp::Connection &conn, vector<tiplocs_t> &tiplocInsert, vector<string> &tiplocDelete);
		static void runAssociation(mysqlpp::Connection &conn, vector<associations_t> &associationInsert, vector<int> &associationDelete);
		static void runSchedules(mysqlpp::Connection &conn, vector<locations_t> &locationInsert, vector<locations_change_t> &locationsChangeInsert, vector<int> &scheduleDelete);
		static void runSchedulesStpCancel(mysqlpp::Connection &conn, vector<CIFRecordNRBS *> &scheduleSTPCancelDelete, vector<CIFRecordNRBS *> &scheduleSTPCancelInsert);
		
		static int findIDForService(mysqlpp::Connection &conn, CIFRecordNRBS *s, bool exact, bool removeDoesntRunOn, bool noDateTo);
		static void deleteService(mysqlpp::Connection &conn, int id);
		static void deleteSTPServiceCancellation(mysqlpp::Connection &conn, int id, string cancelFrom);
		
		static int findIDForAssociation(mysqlpp::Connection &conn, CIFRecordNRAA *a, bool exact, bool removeDoesntRunOn, bool noDateTo);
		static void deleteAssociation(mysqlpp::Connection &conn, int id);
		static void deleteSTPAssociationCancellation(mysqlpp::Connection &conn, int id, string cancelFrom);
};
