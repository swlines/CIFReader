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
using namespace std;

class CIFRecord {
	public:
		virtual unsigned getRecordType(){return 99;};
		virtual ~CIFRecord(){};
		
	protected:
		static string convertYYYYMMDDtoSQL(string date);
		static string convertYYMMDDtoSQL(string date);
		static string convertDDMMYYtoSQL(string date);
};
