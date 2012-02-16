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

#define STRINGTOUPPER " ()&.'"

#include <string>
#include "CIFRecord.h"
#include <boost/algorithm/string.hpp>
using namespace std;
using namespace boost;

string CIFRecord::convertYYMMDDtoSQL(string date) {
	string output = "";
	
	trim(date);
	
	if(date == "999999") {
		return "9999-12-31";
	}
	
	if(date == "") {
		return output;
	}
	 
	int year = atoi(date.substr(0, 2).c_str());
	 
	if(year >= 0 && year <= 59) {
		output += "20";
	}
	else if(year >= 60 && year <= 99) {
		output += "19";
	}
	 
	output += date.substr(0, 2); // year
	output += "-";
	output += date.substr(2, 2); // month
	output += "-";
	output += date.substr(4, 2); // date
 
	return output;
}

string CIFRecord::convertDDMMYYtoSQL(string date) {
	string output = "";
	 
	trim(date);
	
	if(date == "999999") {
		return "9999-12-31";
	}
	
	if(date == "") {
		return output;
	}
	
	 int year = atoi(date.substr(4, 2).c_str());
	 if(year >= 0 && year <= 59) {
	 	output += "20";
	 }
	 else if(year >= 60 && year <= 99) {
	 	output += "19";
	 }
	 
	 output += date.substr(4, 2); // year
	 output += "-";
	 output += date.substr(2, 2); // month
	 output += "-";
	 output += date.substr(0, 2); // date
	 
	 return output;
}

string CIFRecord::convertYYYYMMDDtoSQL(string date) {
	string output = "";
	 
	trim(date);
	if(date == "99999999") {
		return "9999-12-31";
	}
	
	if(date == "") {
		return output;
	}
	 	 
	output  = date.substr(0, 4); // year
	output += "-";
	output += date.substr(4, 2); // month
	output += "-";
	output += date.substr(6, 2); // date
	 
	return output;
}

string CIFRecord::stringToLocationTitleCase(string loc) {
	transform(loc.begin(), loc.end(), loc.begin(), ::tolower);
	
	loc[0] = toupper(loc[0]);
	size_t f;
	
	f = loc.find_first_of(STRINGTOUPPER);
	while(f != string::npos) {
		if(int(f) != int(loc.length()-1)) {
			loc[int(f)+1] = toupper(loc[int(f)+1]);
		}
		
		f = loc.find_first_of(STRINGTOUPPER, f+1);
	}
	
	return loc;
}