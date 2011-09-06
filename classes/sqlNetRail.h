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

#include <mysql++.h>
#include <ssqls.h>

sql_create_33(schedules, 1, 33,
	mysqlpp::sql_varchar, uuid,
	mysqlpp::sql_varchar, train_uid, 
	mysqlpp::sql_date, date_from,
	mysqlpp::sql_date, date_to,
	mysqlpp::sql_char, runs_mo,
	mysqlpp::sql_char, runs_tu,
	mysqlpp::sql_char, runs_we,
	mysqlpp::sql_char, runs_th,
	mysqlpp::sql_char, runs_fr,
	mysqlpp::sql_char, runs_sa,
	mysqlpp::sql_char, runs_su,
	mysqlpp::sql_varchar, bank_hol,
	mysqlpp::sql_varchar, status,
	mysqlpp::sql_varchar, category,
	mysqlpp::sql_varchar, train_identity,
	mysqlpp::sql_varchar, headcode,
	mysqlpp::sql_varchar, service_code,
	mysqlpp::sql_varchar, portion_id,
	mysqlpp::sql_varchar, power_type,
	mysqlpp::sql_varchar, timing_load,
	mysqlpp::sql_varchar, speed,
	mysqlpp::sql_varchar, operating_characteristics,
	mysqlpp::sql_varchar, train_class,
	mysqlpp::sql_varchar, sleepers,
	mysqlpp::sql_varchar, reservations,
	mysqlpp::sql_varchar, catering_code,
	mysqlpp::sql_varchar, service_branding,
	mysqlpp::sql_varchar, stp_indicator,
	mysqlpp::sql_varchar, uic_code,
	mysqlpp::sql_varchar, atoc_code,
	mysqlpp::sql_varchar, ats_code,
	mysqlpp::sql_varchar, rsid,
	mysqlpp::sql_varchar, data_source);


sql_create_6(tiplocs, 1, 6,
	mysqlpp::sql_varchar, tiploc,
	mysqlpp::sql_varchar, nalco,
	mysqlpp::sql_varchar, tps_description,
	mysqlpp::sql_varchar, stanox,
	mysqlpp::sql_varchar, crs,
	mysqlpp::sql_varchar, description);
	
sql_create_17(locations, 1, 17,
	mysqlpp::sql_varchar, uuid,
	mysqlpp::sql_int, location_order,
	mysqlpp::sql_char, location_type,
	mysqlpp::sql_varchar, tiploc_code,
	mysqlpp::sql_varchar, tiploc_instance,
	mysqlpp::sql_varchar, arrival,
	mysqlpp::sql_varchar, public_arrival,
	mysqlpp::sql_varchar, pass,
	mysqlpp::sql_varchar, departure,
	mysqlpp::sql_varchar, public_departure,
	mysqlpp::sql_varchar, platform,
	mysqlpp::sql_varchar, line,
	mysqlpp::sql_varchar, path,
	mysqlpp::sql_varchar, engineering_allowance,
	mysqlpp::sql_varchar, pathing_allowance,
	mysqlpp::sql_varchar, performance_allowance,
	mysqlpp::sql_varchar, activity);
