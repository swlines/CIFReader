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
#include <boost/algorithm/string.hpp>
#ifndef _CIF_RECORD_INC
#define _CIF_RECORD_INC
	#include "../CIFRecord.h"
#endif
using namespace std;
using namespace boost;

class CIFRecordNRTITA : public CIFRecord {
	public:
		unsigned getRecordType();
		CIFRecordNRTITA(string rec);
		~CIFRecordNRTITA();
		string record_type, tiploc_code, nlc, tps_desc, stanox, crs, capri_desc, old_tiploc;
};

unsigned CIFRecordNRTITA::getRecordType() { 
	if(record_type == "I") {
		return 1;
	}
	else if(record_type == "A") {
		if(old_tiploc == "") {
			return 2;
		}
		else {
			return 3;
		}
	}
	else return 99;
}

CIFRecordNRTITA::CIFRecordNRTITA(string rec) {
	nlc				= rec.substr(11, 6);
	tps_desc		= rec.substr(18,26);
	stanox			= rec.substr(44, 5);
	crs				= rec.substr(53, 3);
	capri_desc		= rec.substr(56,16);
	
	if(rec.substr(0,2) == "TI") {
		record_type = "I";
		old_tiploc = "";
		tiploc_code = rec.substr(2,  7);
	}
	else if(rec.substr(0,2) == "TA") {
		record_type = "A";
		string newTiploc = rec.substr(72, 7);
		trim(newTiploc);
		
		if(newTiploc == "") { 
			tiploc_code = rec.substr(2,7);
			old_tiploc = "";
		}
		else {
			old_tiploc = rec.substr(2,7);
			tiploc_code = newTiploc;
		}		
	}
	else { // this should never occur, but nevertheless, the tiploc_code
		old_tiploc = "";
		tiploc_code = rec.substr(2,  7);
	}
	
	trim(nlc);
	trim(crs);
	trim(tps_desc);
	trim(capri_desc);
	trim(tiploc_code);
	trim(old_tiploc);
}

CIFRecordNRTITA::~CIFRecordNRTITA() {
	tiploc_code.clear();
	nlc.clear();
	tps_desc.clear();
	stanox.clear();
	crs.clear();
	capri_desc.clear();
	old_tiploc.clear();
}
