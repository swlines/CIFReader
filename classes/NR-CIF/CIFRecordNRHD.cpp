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
#include <map>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
using namespace std;
using namespace boost;

class CIFRecordNRHD : public CIFRecord {
	public:
		unsigned getRecordType();
		CIFRecordNRHD(string rec);
		~CIFRecordNRHD();
		string mainframe_id, date_extract, time_extract, curr_file_ref, last_file_ref, update_type, extract_start, extract_end, mainframe_user, extract_date;
};

unsigned CIFRecordNRHD::getRecordType() { 
	return 00;
}

CIFRecordNRHD::CIFRecordNRHD(string rec) {
	mainframe_id 	= rec.substr(2, 20);
	date_extract	= CIFRecord::convertDDMMYYtoSQL(rec.substr(22, 6));
	time_extract	= rec.substr(28, 4);
	curr_file_ref	= rec.substr(32, 7);
	last_file_ref	= rec.substr(39, 7);
	update_type		= rec.substr(46, 1);
	extract_start	= CIFRecord::convertDDMMYYtoSQL(rec.substr(48, 6));
	extract_end		= CIFRecord::convertDDMMYYtoSQL(rec.substr(54, 6));
	
	// run regex check for main_frame user and extract date
	regex mainframe_expr("TPS.U(.{6}).PD(.{6})");
	bool match = regex_match(mainframe_id, mainframe_expr);
	
	if(match) {
		mainframe_user 	= mainframe_id.substr(5,  6);
		extract_date	= CIFRecord::convertYYMMDDtoSQL(mainframe_id.substr(14, 6));
	}
	else {
		throw -1;
	}
}

CIFRecordNRHD::~CIFRecordNRHD() {
	mainframe_id.clear();
	date_extract.clear();
	time_extract.clear();
	curr_file_ref.clear();
	last_file_ref.clear();
	update_type.clear();
	extract_start.clear();
	extract_end.clear();
	mainframe_user.clear();
	extract_date.clear();
}
