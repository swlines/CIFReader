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
