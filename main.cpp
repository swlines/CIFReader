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

#include "classes/CIF.cpp"
#include <iostream>
#include <vector>
#include <string>
#include <mysql++.h>

using namespace std;

int main(int argc, char *argv[]) {	
	cout << "CIF Explorer - Copyright 2011 Tom Cairns" << endl << 	
			"This program comes with ABSOLUTELY NO WARRANTY. " << endl <<
			"This is free software, and you are welcome to redistribute it " << endl << 
			"under certain conditions, see LICENCE." << endl;

	if(argc == 2) {
		CIF::processCIFFile(argv[1]);
		cout << endl << endl;
	}
	else {
		cout << "Did not specify valid format." << endl << "Usage: " << argv[0] << " file_location" << endl;
	}
	
	return 0;
}
