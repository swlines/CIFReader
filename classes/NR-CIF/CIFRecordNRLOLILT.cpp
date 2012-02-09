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

#include "CIFRecordNRLOLILT.h"

#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

unsigned CIFRecordNRLOLILT::getRecordType() { 
	return 7;
}

CIFRecordNRLOLILT::CIFRecordNRLOLILT(string rec) {
	record_type 			= rec.substr(0,  2);
	tiploc					= rec.substr(2,  7);
	tiploc_instance			= rec.substr(9,  1);
	
	if(record_type == "LO") {
		departure				= rec.substr(10, 5);
		public_departure		= rec.substr(15, 4);
		platform				= rec.substr(19, 3);
		line					= rec.substr(22, 3);
		engineering_allowance	= rec.substr(25, 2);
		pathing_allowance		= rec.substr(27, 2);
		activity				= rec.substr(29,12);
		performance_allowance	= rec.substr(41, 2);
	}
	else if(record_type == "LI") {
		arrival					= rec.substr(10, 5);
		departure				= rec.substr(15, 5);
		pass					= rec.substr(20, 5);
		public_arrival			= rec.substr(25, 4);
		public_departure		= rec.substr(29, 4);
		platform				= rec.substr(33, 3);
		line					= rec.substr(36, 3);
		path					= rec.substr(39, 3);
		activity				= rec.substr(42,12);
		engineering_allowance	= rec.substr(54, 2);
		pathing_allowance		= rec.substr(56, 2);
		performance_allowance	= rec.substr(58, 2);
	}
	else if(record_type == "LT") {
		arrival					= rec.substr(10, 5);
		public_arrival			= rec.substr(15, 4);
		platform				= rec.substr(19, 3);
		path					= rec.substr(22, 3);
		activity				= rec.substr(25,12);
	}
	
	trim(tiploc);
	trim(tiploc_instance);

	trim(arrival);
	trim(departure);
	trim(pass);
	trim(public_arrival);
	trim(public_departure);
	
	trim(platform);
	trim(line);
	trim(path);
	trim(activity);
	trim(engineering_allowance);
	trim(pathing_allowance);
	trim(performance_allowance);
	
	if(public_arrival == "0000" || public_arrival == "0000 ") {
		public_arrival = "";
	}
	
	if(public_departure == "0000" || public_departure == "0000 ") { 
		public_departure = "";
	}
	
	// set up public call
	public_call = (public_arrival != "" || public_departure != "");
	actual_call = (arrival != "" || departure != "");
	
	//now create the ordertime field...
	if(pass != "") {
		order_time = pass;
	}
	else if(departure != "") { // departure time...
		order_time = departure;
	}
	else { // arrival time...
		order_time = arrival;
	}
}

CIFRecordNRLOLILT::~CIFRecordNRLOLILT() {
	record_type.clear();
	tiploc.clear();
	tiploc_instance.clear();
	arrival.clear();
	public_arrival.clear();
	pass.clear();
	departure.clear();
	public_departure.clear();
	platform.clear();
	line.clear();
	path.clear();
	engineering_allowance.clear();
	pathing_allowance.clear();
	performance_allowance.clear();
	activity.clear();
}
