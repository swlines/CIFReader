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
	trim(engineering_allowance);
	trim(pathing_allowance);
	trim(performance_allowance);
	
	if(public_arrival == "0000" || public_arrival == "0000 ") {
		public_arrival = "";
	}
	
	if(public_departure == "0000" || public_departure == "0000 ") { 
		public_departure = "";
	}
	
	// set up activities as 0
	act_a = 0;
	act_ae = 0;
	act_bl = 0;
	act_c = 0;
	act_d = 0;
	act_minusd = 0;
	act_e = 0;
	act_g = 0;
	act_h = 0;
	act_hh = 0;
	act_k = 0;
	act_kc = 0;
	act_ke = 0;
	act_kf = 0;
	act_ks = 0;
	act_l = 0;
	act_n = 0;
	act_op = 0;
	act_or = 0;
	act_pr = 0;
	act_r = 0;
	act_rm = 0;
	act_rr = 0;
	act_s = 0;
	act_t = 0;
	act_minust = 0;
	act_tb = 0;
	act_tf = 0;
	act_ts = 0;
	act_tw = 0;
	act_u = 0;
	act_minusu = 0;
	act_w = 0;
	act_x = 0;
	string act;
	
	// process activity
	for(int i = 0; i < 12; i=i+2) {
		act = activity.substr(i, 2);
		
		if(act == "A ") { act_a = 1; }
		else if(act == "AE") { act_ae = 1; }
		else if(act == "BL") { act_bl = 1; }
		else if(act == "C ") { act_c = 1; }
		else if(act == "D ") { act_d = 1; }
		else if(act == "-D") { act_minusd = 1; }
		else if(act == "E ") { act_e = 1; }
		else if(act == "G ") { act_g = 1; }
		else if(act == "H ") { act_h = 1; }
		else if(act == "HH") { act_hh = 1; }
		else if(act == "K ") { act_k = 1; }
		else if(act == "KC") { act_kc = 1; }
		else if(act == "KE") { act_ke = 1; }
		else if(act == "KF") { act_kf = 1; }
		else if(act == "KS") { act_ks = 1; }
		else if(act == "L ") { act_l = 1; }
		else if(act == "N ") { act_n = 1; }
		else if(act == "OP") { act_op = 1; }
		else if(act == "OR") { act_or = 1; }
		else if(act == "PR") { act_pr = 1; }
		else if(act == "R ") { act_r = 1; }
		else if(act == "RM") { act_rm = 1; }
		else if(act == "RR") { act_rr = 1; }
		else if(act == "S ") { act_s = 1; }
		else if(act == "T ") { act_t = 1; }
		else if(act == "-T") { act_minust = 1; }
		else if(act == "TB") { act_tb = 1; }
		else if(act == "TF") { act_tf = 1; }
		else if(act == "TS") { act_ts = 1; }
		else if(act == "TW") { act_tw = 1; }
		else if(act == "U ") { act_u = 1; }
		else if(act == "-U") { act_minusu = 1; }
		else if(act == "W ") { act_w = 1; }
		else if(act == "X ") { act_x = 1; }	
	}
	
	// set up public call
	public_call = ((public_arrival != "" || public_departure != "") && act_n = 0);
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
	
	trim(activity);	
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
