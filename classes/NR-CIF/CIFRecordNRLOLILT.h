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

class CIFRecordNRLOLILT : public CIFRecord {
	public:
		unsigned getRecordType();
		CIFRecordNRLOLILT(string rec);
		~CIFRecordNRLOLILT();
		string record_type, tiploc, tiploc_instance, arrival, public_arrival, pass, departure, public_departure, platform, line, path, engineering_allowance, pathing_allowance, performance_allowance, activity, order_time;
		
		bool public_call, actual_call;
		
		bool act_a, act_ae, act_bl, act_c, act_d, act_minusd, act_e, act_g, act_h, act_hh, act_k, act_kc, act_ke, act_kf, act_ks, act_l, act_n, act_op, act_or, act_pr, act_r, act_rm, act_rr, act_s, act_t, act_minust, act_tb, act_tf, act_ts, act_tw, act_u, act_minusu, act_w, act_x;
};
