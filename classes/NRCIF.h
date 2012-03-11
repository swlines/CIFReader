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

#ifndef _CIFREC_INC
	#define _CIFREC_INC 1
	#include "CIFRecord.h"
#endif

#ifndef _NRCIF_HEADERS_INC
	#define _NRCIF_HEADERS_INC 1
	#include "NR-CIF/CIFRecordNRHD.h"
	#include "NR-CIF/CIFRecordNRTITA.h"
	#include "NR-CIF/CIFRecordNRTD.h"
	#include "NR-CIF/CIFRecordNRBS.h"
	#include "NR-CIF/CIFRecordNRLOLILT.h"
	#include "NR-CIF/CIFRecordNRAA.h"
	#include "NR-CIF/CIFRecordNRCR.h"
#endif

using namespace std;

class NRCIF {
	public:
		static bool processFile(mysqlpp::Connection &conn, const char* filePath);
		static void deleteService(mysqlpp::Connection &conn, int id);
		static void deleteAssociation(mysqlpp::Connection &conn, int id);
		
		#ifdef GLA
			static void calculateGLA(mysqlpp::Connection &conn);
		#endif
		
	private:
		static CIFRecord* processLine(string record);
		static void runTiploc(mysqlpp::Connection &conn, vector<CIFRecordNRTITA *> &tiplocInsert, vector<string> &tiplocDelete);
		static void runSchedulesStpCancel(mysqlpp::Connection &conn, CIFRecordNRHD *header, vector<CIFRecordNRBS *> &scheduleSTPCancelDelete, vector<CIFRecordNRBS *> &scheduleSTPCancelInsert);
		static void runAssociationsStpCancel(mysqlpp::Connection &conn, CIFRecordNRHD *header, vector<CIFRecordNRAA *> &associationSTPCancelDelete, vector<CIFRecordNRAA *> &associationSTPCancelInsert);
		
		static int findIDForService(mysqlpp::Connection &conn, CIFRecordNRBS *s, CIFRecordNRHD *h, bool exact, bool removeDoesntRunOn, bool noDateTo);
		static int findIDForServiceStpCancel(mysqlpp::Connection &conn, CIFRecordNRBS *s);
		static void deleteSTPServiceCancellation(mysqlpp::Connection &conn, CIFRecordNRBS *s);
		
		static int findIDForAssociation(mysqlpp::Connection &conn, CIFRecordNRAA *a, CIFRecordNRHD *h, bool exact, bool removeDoesntRunOn, bool noDateTo);
		static int findIDForAssociationStpCancel(mysqlpp::Connection &conn, CIFRecordNRAA *a);
		static void deleteSTPAssociationCancellation(mysqlpp::Connection &conn, CIFRecordNRAA *a);
};
