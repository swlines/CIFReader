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
using namespace std;
using namespace boost;

class CIFRecordNRTD : public CIFRecord {
	public:
		unsigned getRecordType();
		CIFRecordNRTD(string rec);
		~CIFRecordNRTD();
		string tiploc_code, nlc, tps_desc, stanox, crs, capri_desc, old_tiploc;
};

unsigned CIFRecordNRTD::getRecordType() { 
	return 4;
}

CIFRecordNRTD::CIFRecordNRTD(string rec) {
	tiploc_code = rec.substr(2,  7);

	trim(tiploc_code);
}

CIFRecordNRTD::~CIFRecordNRTD() {
	tiploc_code.clear();
}
