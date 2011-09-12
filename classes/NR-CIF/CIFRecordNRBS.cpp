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
#include <uuid/uuid.h>
#ifndef _CIF_RECORD_INC
#define _CIF_RECORD_INC
	#include "../CIFRecord.h"
#endif
using namespace std;
using namespace boost;

class CIFRecordNRBS : public CIFRecord {
	public:
		unsigned getRecordType();
		void mergeWithBX(string line);
		CIFRecordNRBS(string rec);
		~CIFRecordNRBS();
		string unique_id, transaction_type, uid, date_from, date_to, runs_mo, runs_tu, runs_we, runs_th, runs_fr, runs_sa, runs_su, bank_holiday, status, category, train_identity, headcode, service_code, portion_id, power_type, timing_load, speed, operating_characteristics, train_class, sleepers, reservations, catering_code, service_branding, stp_indicator, uic_code, atoc_code, ats_code, rsid, data_source;
};

unsigned CIFRecordNRBS::getRecordType() { 
	return 6;
}

CIFRecordNRBS::CIFRecordNRBS(string rec) {
	transaction_type 	= rec.substr(2,  1);
	
	// do not need to generate a uuid if we are deleting a file...
	if(transaction_type == "N" || transaction_type == "R") {
		char uuidBuffer[36];
		uuid_t uuid;
		uuid_generate(uuid);
		uuid_unparse(uuid, uuidBuffer);
	
		stringstream ss;
		ss << uuidBuffer;
		ss >> unique_id;
	}
	
	// needed to stop exception being thrown on short BS Delete record if it appears
	try {
		uid					= rec.substr(3,  6);
		date_from			= CIFRecord::convertYYMMDDtoSQL(rec.substr(9,  6));
		date_to				= CIFRecord::convertYYMMDDtoSQL(rec.substr(15, 6));
		runs_mo				= rec.substr(21, 1);
		runs_tu				= rec.substr(22, 1);
		runs_we				= rec.substr(23, 1);
		runs_th				= rec.substr(24, 1);
		runs_fr				= rec.substr(25, 1);
		runs_sa				= rec.substr(26, 1);
		runs_su				= rec.substr(27, 1);
		bank_holiday		= rec.substr(28, 1);
		status				= rec.substr(29, 1);
		category			= rec.substr(30, 2);
		train_identity		= rec.substr(32, 4);
		headcode			= rec.substr(36, 4);
		service_code		= rec.substr(41, 8);
		portion_id			= rec.substr(49, 1);
		power_type			= rec.substr(50, 3);
		timing_load			= rec.substr(53, 4);
		speed				= rec.substr(57, 3);
		operating_characteristics = rec.substr(60, 6);
		train_class			= rec.substr(66, 1);
		sleepers			= rec.substr(67, 1);
		reservations		= rec.substr(68, 1);
		catering_code		= rec.substr(70, 4);
		service_branding	= rec.substr(74, 4);
		stp_indicator		= rec.substr(79, 1);
	}
	catch(out_of_range& oor){}
	
	// if a cancel, then trimming the runs_mo, etc, records is helpful as it helps
	// when searching for a uuid.
	if(stp_indicator == "C" || transaction_type == "D") {
		if(runs_mo == "0" || runs_mo == " ") trim(runs_mo);
		if(runs_tu == "0" || runs_tu == " ") trim(runs_tu);
		if(runs_we == "0" || runs_we == " ") trim(runs_we);
		if(runs_th == "0" || runs_th == " ") trim(runs_th);
		if(runs_fr == "0" || runs_fr == " ") trim(runs_fr);
		if(runs_sa == "0" || runs_sa == " ") trim(runs_sa);
		if(runs_su == "0" || runs_su == " ") trim(runs_su);
	}
	
	trim(bank_holiday);
	trim(category);
	trim(headcode);
	trim(portion_id);
	trim(operating_characteristics);
	trim(sleepers);
	trim(reservations);
	trim(catering_code);
	trim(service_branding);
	trim(stp_indicator);
}

void CIFRecordNRBS::mergeWithBX(string rec) {
	if(rec.substr(0, 2) != "BX") throw 5;
	
	uic_code 			= rec.substr(6,  5);
	atoc_code			= rec.substr(11, 2);
	ats_code			= rec.substr(13, 1);
	rsid				= rec.substr(14, 8);
	data_source			= rec.substr(22, 1);
	
	trim(uic_code);
	trim(atoc_code);
	trim(rsid);
	trim(data_source);
}

CIFRecordNRBS::~CIFRecordNRBS() {
	transaction_type.clear();
	uid.clear();
	date_from.clear();
	date_to.clear();
	runs_mo.clear();
	runs_tu.clear();
	runs_we.clear();
	runs_th.clear();
	runs_fr.clear();
	runs_sa.clear();
	runs_su.clear();
	bank_holiday.clear();
	status.clear();
	category.clear();
	train_identity.clear();
	headcode.clear();
	service_code.clear();
	portion_id.clear();
	power_type.clear();
	power_type.clear();
	timing_load.clear();
	speed.clear();
	operating_characteristics.clear();
	train_class.clear();
	sleepers.clear();
	reservations.clear();
	catering_code.clear();
	service_branding.clear();
	stp_indicator.clear();
	uic_code.clear();
	atoc_code.clear();
	ats_code.clear();
	rsid.clear();
	data_source.clear();
}
