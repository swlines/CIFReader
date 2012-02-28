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

#ifndef _CIFREC_INC
	#define _CIFREC_INC
	#include "../CIFRecord.h"
#endif

#include <string>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;

class CIFRecordNRBS : public CIFRecord {
	public:
		unsigned getRecordType();
		void mergeWithBX(string line);
		CIFRecordNRBS(string rec);
		~CIFRecordNRBS();
		string transaction_type, uid, unique_id, date_from, date_to, runs_mo, runs_tu, runs_we, runs_th, runs_fr, runs_sa, runs_su, bank_holiday, status, category, train_identity, headcode, service_code, portion_id, power_type, timing_load, speed, operating_characteristics, train_class, sleepers, reservations, catering_code, service_branding, stp_indicator, uic_code, atoc_code, ats_code, rsid, data_source;
		
		bool oc_b, oc_c, oc_d, oc_e, oc_g, oc_m, oc_p, oc_q, oc_r, oc_s, oc_y, oc_z;
		
		int train, bus, ship, passenger;
};
