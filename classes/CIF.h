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
#include <mysql++.h>
#include "CIFRecord.cpp"

#include "NR-CIF/CIFRecordNRHD.cpp"
#include "NR-CIF/CIFRecordNRTITA.cpp"
#include "NR-CIF/CIFRecordNRTD.cpp"
#include "NR-CIF/CIFRecordNRBS.cpp"
#include "NR-CIF/CIFRecordNRLOLILT.cpp"
#include "NR-CIF/CIFRecordNRAA.cpp"

using namespace std;

class CIF {
	public:
		static void processCIFFile(const char* filePath);
		
	private:
		static streampos fileSize(const char* filePath);
		static CIFRecord* processNRCIFLine(string record);
		
		static string findUUIDForService(mysqlpp::Connection &conn, CIFRecordNRBS *s, bool exact);
		static void deleteService(mysqlpp::Connection &conn, string uuid);
		static void deleteSTPServiceCancellation(mysqlpp::Connection &conn, string uuid, string cancelFrom, string cancelTo);
		
		static string findUUIDForAssociation(mysqlpp::Connection &conn, CIFRecordNRAA *a, bool exact);
		static void deleteAssociation(mysqlpp::Connection &conn, string uuid);
};
