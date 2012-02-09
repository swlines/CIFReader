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

#include "CIFRecordNRAA.h"
#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

unsigned CIFRecordNRAA::getRecordType() { 
	return 5;
}

CIFRecordNRAA::CIFRecordNRAA(string rec) {
	transaction_type 	= rec.substr(2,  1);
	
	// catch any out of range errors, meaning record has finished
	try {
		main_train_uid 			= rec.substr(3,  6);
		assoc_train_uid			= rec.substr(9,  6);
		date_from				= CIFRecord::convertYYMMDDtoSQL(rec.substr(15, 6));
		date_to					= CIFRecord::convertYYMMDDtoSQL(rec.substr(21, 6));
		assoc_mo				= rec.substr(27, 1);
		assoc_tu				= rec.substr(28, 1);
		assoc_we				= rec.substr(29, 1);
		assoc_th				= rec.substr(30, 1);
		assoc_fr				= rec.substr(31, 1);
		assoc_sa				= rec.substr(32, 1);
		assoc_su				= rec.substr(33, 1);
		category				= rec.substr(34, 2);
		date_indicator			= rec.substr(36, 1);
		location				= rec.substr(37, 7);
		base_location_suffix	= rec.substr(44, 1);
		assoc_location_suffix	= rec.substr(45, 1);
		assoc_type				= rec.substr(47, 1);
		stp_indicator			= rec.substr(79, 1);
	}
	catch(out_of_range& oor){}
	
	// if a cancel, then trimming the assoc_mo, etc, records is helpful as it helps
	// when searching for a uuid.
	if(stp_indicator == "C" || transaction_type == "D") {
		if(assoc_mo == "0" || assoc_mo == " ") trim(assoc_mo);
		if(assoc_tu == "0" || assoc_tu == " ") trim(assoc_tu);
		if(assoc_we == "0" || assoc_we == " ") trim(assoc_we);
		if(assoc_th == "0" || assoc_th == " ") trim(assoc_th);
		if(assoc_fr == "0" || assoc_fr == " ") trim(assoc_fr);
		if(assoc_sa == "0" || assoc_sa == " ") trim(assoc_sa);
		if(assoc_su == "0" || assoc_su == " ") trim(assoc_su);
	}
	
	trim(category);
	trim(date_indicator);
	trim(location);
	trim(base_location_suffix);
	trim(assoc_location_suffix);
	trim(assoc_type);
	trim(stp_indicator);
}

CIFRecordNRAA::~CIFRecordNRAA() {
	transaction_type.clear();
	main_train_uid.clear();
	assoc_train_uid.clear();
	date_from.clear();
	date_to.clear();
	assoc_mo.clear();
	assoc_tu.clear();
	assoc_we.clear();
	assoc_th.clear();
	assoc_fr.clear();
	assoc_sa.clear();
	assoc_su.clear();
	category.clear();
	date_indicator.clear();
	location.clear();
	base_location_suffix.clear();
	assoc_location_suffix.clear();
	assoc_type.clear();
	stp_indicator.clear();
}
