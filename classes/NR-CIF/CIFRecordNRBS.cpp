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

#include "CIFRecordNRBS.h"

#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

unsigned CIFRecordNRBS::getRecordType() { 
	return 6;
}

CIFRecordNRBS::CIFRecordNRBS(string rec) {
	transaction_type 	= rec.substr(2,  1);
	
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
	trim(sleepers);
	trim(reservations);
	trim(catering_code);
	trim(service_branding);
	trim(stp_indicator);

	// unique id formed from schedule uid, date from and stp indicator
	unique_id = uid + rec.substr(9,  6) + stp_indicator;
	
	bus = 0;
	train = 1;
	ship = 0;
	passenger = 0;
	
	// bus
	if(category == "BR" || category == "BS") {
		bus = 1;
		train = 0;
	}
	
	// ship
	else if(status == "S" || status == "4") {
		ship = 1;
		train = 0;
	}
	
	// passenger
	if(bus == 1 || ship == 1 || category == "OL" || category == "OO" || category == "XC" || category == "XX" || category == "XZ") {
		passenger = 1;
	}
	
	oc_b = 0;
	oc_c = 0;
	oc_d = 0;
	oc_e = 0;
	oc_g = 0;
	oc_m = 0;
	oc_p = 0;
	oc_q = 0;
	oc_r = 0;
	oc_s = 0;
	oc_y = 0;
	oc_z = 0;
	string oc;
	
	for(unsigned int i = 0; i < operating_characteristics.length(); i++) {
		oc = operating_characteristics.substr(i, 1);
		
		if(oc == "B") { oc_b = 1; }
		else if(oc == "C") { oc_c = 1; }
		else if(oc == "D") { oc_d = 1; }
		else if(oc == "E") { oc_e = 1; }
		else if(oc == "G") { oc_g = 1; }
		else if(oc == "M") { oc_m = 1; }
		else if(oc == "P") { oc_p = 1; }
		else if(oc == "Q") { oc_q = 1; }
		else if(oc == "R") { oc_r = 1; }
		else if(oc == "S") { oc_s = 1; }
		else if(oc == "Y") { oc_y = 1; }
		else if(oc == "Z") { oc_z = 1; }
	}
	
	trim(operating_characteristics);
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
