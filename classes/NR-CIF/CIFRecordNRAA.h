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

class CIFRecordNRAA : public CIFRecord {
	public:
		unsigned getRecordType();
		CIFRecordNRAA(string rec);
		~CIFRecordNRAA();
		string transaction_type, main_train_uid, assoc_train_uid, date_from, date_to, assoc_mo, assoc_tu, assoc_we, assoc_th, assoc_fr, assoc_sa, assoc_su, category, date_indicator, location, base_location_suffix, assoc_location_suffix, assoc_type, stp_indicator;
};
