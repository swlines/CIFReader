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

class CIFRecordNRCR : public CIFRecord {
	public:
		unsigned getRecordType();
		CIFRecordNRCR(string rec);
		~CIFRecordNRCR();
		string tiploc, tiploc_suffix, category, train_identity, headcode, service_code, portion_id, power_type, timing_load, speed, operating_characteristics, train_class, sleepers, reservations, catering_code, service_branding, uic_code, rsid;
};

unsigned CIFRecordNRCR::getRecordType() { 
	return 8;
}

CIFRecordNRCR::CIFRecordNRCR(string rec) {
	tiploc 						= rec.substr(2,  7);
	tiploc_suffix				= rec.substr(7,  1);
	category					= rec.substr(8,  2);
	train_identity				= rec.substr(10, 4);
	headcode					= rec.substr(14, 4);
	service_code				= rec.substr(19, 8);
	portion_id					= rec.substr(27, 1);
	power_type					= rec.substr(28, 3);
	timing_load					= rec.substr(31, 4);
	speed						= rec.substr(35, 3);
	operating_characteristics	= rec.substr(38, 6);
	train_class					= rec.substr(44, 1);
	sleepers					= rec.substr(45, 1);
	reservations				= rec.substr(46, 1);
	catering_code				= rec.substr(47, 4);
	service_branding			= rec.substr(48, 4);
	uic_code					= rec.substr(56, 5);
	rsid						= rec.substr(61, 8);
	
	trim(category);
	trim(headcode);
	trim(portion_id);
	trim(operating_characteristics);
	trim(sleepers);
	trim(reservations);
	trim(catering_code);
	trim(service_branding);
	trim(uic_code);
	trim(rsid);
}

CIFRecordNRCR::~CIFRecordNRCR() {
	tiploc.clear();
	tiploc_suffix.clear();
	category.clear();
	train_identity.clear();
	headcode.clear();
	service_code.clear();
	portion_id.clear();
	power_type.clear();
	timing_load.clear();
	speed.clear();
	operating_characteristics.clear();
	train_class.clear();
	sleepers.clear();
	reservations.clear();
	catering_code.clear();
	service_branding.clear();
	uic_code.clear();
	rsid.clear();
}
