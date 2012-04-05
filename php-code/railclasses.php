<?php

class railServiceSearch {
	
	// change your settings to what you want them to use
	const db_database = 'timetables'; // fixed in sql
	const db_username = '';
	const db_password = '';
	const db_location = '';
	const overlay 	  = true;
		
	public static $maxPeriod = 11;

	public static function getPassengerCategories() {
		$obj = new stdClass;
		$obj->categories = array('OL', 'OO', 'XC', 'XX', 'XZ');
		$obj->status = array();
		return $obj;
	}
	
	public static function getBusCategories() { 
		$obj = new stdClass;
		$obj->categories = array('BR', 'BS');
		$obj->status = array();
		return $obj;
	}
	
	public static function getShipCategories() {
		$obj = new stdClass;
		$obj->categories = array();
		$obj->status = array('S', '4');
		return $obj;
	}
	
	// do not edit below this line - unless you want to mod how it dies, then edit all the lines
	// mentioning die( ... :)
	
	private $db;
	private $query_result;
	
	public function __construct()
	{   			
		// establish connection to the database
		$this->db = @mysqli_connect(self::db_location, self::db_username, self::db_password, self::db_database) or die(mysqli_connect_error());
		
		// check if connection exists and perform the initial data modification
		if($this->db) 
		{
			mysqli_query($this->db, "SET NAMES 'UTF-8'");  
			if(date_default_timezone_get() != 'UTC') date_default_timezone_set('UTC'); 
		}
	}
	
	public function __destruct() {
		mysqli_close($this->db);
		$this->db = null;
	}
	
	// query functions
	private function query($query = '') 
	{
		// initial check to see if query not blank
		if($query == '')
		{
			die('Query request blank');
		}
		
		$this->query_result = mysqli_query($this->db, $query);
		
		if($this->query_result === false) 
		{
			die(mysqli_error($this->db));
		} 
		elseif($this->query_result === true)
		{
			return true;
		}
		
		if($this->query_result->num_rows == 0 || $this->query_result->num_rows < 0)
		{
			return false;
		}
		else
		{
			return $this->query_result;
		}
	}
	
	private function escape($string) 
	{
		return @mysqli_real_escape_string($this->db, $string);
	}
	
	public static function getMySQLDate($date) {
		$date = new DateTime($date, new DateTimeZone('UTC'));
		return array($date->format('Y-m-d'), substr(strtolower($date->format('D')), 0, 2));
	}
	
	public static function getMySQLDateYMD($date) {
		list($d, $a) = self::getMySQLDate($date);
		return $d;
	}
	
	public static function getMySQLDateText($date) {
		$date = new DateTime($date, new DateTimeZone('UTC'));
		return $date->format('jS F Y');
	}

	public function getLocationTableForDay($tiploc, $date) {
		return $this->buildLocationTable($tiploc, $date);
	}
	
	public static function getPeriodForTime($time) {	
			if($time >= '0000' && $time < '0200') return 0;
		elseif($time >= '0200' && $time < '0400') return 1;
		elseif($time >= '0400' && $time < '0600') return 2;
		elseif($time >= '0600' && $time < '0800') return 3;
		elseif($time >= '0800' && $time < '1000') return 4;
		elseif($time >= '1000' && $time < '1200') return 5;
		elseif($time >= '1200' && $time < '1400') return 6;
		elseif($time >= '1400' && $time < '1600') return 7;
		elseif($time >= '1600' && $time < '1800') return 8;
		elseif($time >= '1800' && $time < '2000') return 9;
		elseif($time >= '2000' && $time < '2200') return 10;
		elseif($time >= '2200') return 11;
	}
	
	public static function getTimeForPeriod($period) {
		switch($period) {
			case 0: return (object)array('start'=>'0000', 'end'=>'0200'); break;
			case 1: return (object)array('start'=>'0200', 'end'=>'0400'); break;
			case 2: return (object)array('start'=>'0400', 'end'=>'0600'); break;
			case 3: return (object)array('start'=>'0600', 'end'=>'0800'); break;
			case 4: return (object)array('start'=>'0800', 'end'=>'1000'); break;
			case 5: return (object)array('start'=>'1000', 'end'=>'1200'); break;
			case 6: return (object)array('start'=>'1200', 'end'=>'1400'); break;
			case 7: return (object)array('start'=>'1400', 'end'=>'1600'); break;
			case 8: return (object)array('start'=>'1600', 'end'=>'1800'); break;
			case 9: return (object)array('start'=>'1800', 'end'=>'2000'); break;
			case 10:return (object)array('start'=>'2000', 'end'=>'2200'); break;
			case 11:return (object)array('start'=>'2200', 'end'=>'2359'); break;
			
			default: return NULL; break;
		}
	}
	
	public static function generateDaysRun($mo, $tu, $we, $th, $fr, $sa, $su) {
		$doesNotOperate = ''; $doesOperate = '';
		if($mo == '0') { $doesNotOperate .= 'M'; } 	else { $doesOperate .= 'M'; }
		if($tu == '0') { $doesNotOperate .= 'T'; }	else { $doesOperate .= 'T'; }
		if($we == '0') { $doesNotOperate .= 'W'; }	else { $doesOperate .= 'W'; }
		if($th == '0') { $doesNotOperate .= 'Th'; }	else { $doesOperate .= 'Th'; }
		if($fr == '0') { $doesNotOperate .= 'F'; }	else { $doesOperate .= 'F'; }
		if($sa == '0') { $doesNotOperate .= 'S'; }	else { $doesOperate .= 'S'; }
		if($su == '0') { $doesNotOperate .= 'Su'; }	else { $doesOperate .= 'Su'; }
		
		if(strlen($doesNotOperate) < strlen($doesOperate)) 
			$op = $doesNotOperate . 'X';
		else
			$op = $doesOperate . 'O';
			
		return $op;
	}
	
	public static function getServiceStatus($st) {
		switch(trim($st)) {
			case 'B': $status = 'Bus (WTT)'; break;
			case 'F': $status = 'Freight (WTT)'; break;
			case 'P': $status = 'Passenger & Parcels (WTT)'; break;
			case 'S': $status = 'Ship (WTT)'; break;
			case 'T': $status = 'Trip (WTT)'; break;
			
			case '1': $status = 'Passenger & Parcels (STP)'; break;
			case '2': $status = 'Freight (STP)'; break;
			case '3': $status = 'Trip (STP)'; break;
			case '4': $status = 'Ship (STP)'; break;
			case '5': $status = 'Bus (STP)'; break;
			
			default: $status = 'Unknown'; break;
		}
		
		return $status;
	}
	
	public static function getServiceCategory($cat) {
		switch(trim($cat)) {
			case 'OL': $category = 'London Underground/Metro'; break;
			case 'OU': $category = 'Unadvertised Ordinary Passenger'; break;
			case 'OO': $category = 'Ordinary Passenger'; break;
			case 'OS': $category = 'Staff Train'; break;
			case 'OW': $category = 'Mixed'; break;
			
			case 'XC': $category = 'Channel Tunnel'; break;
			case 'XD': $category = 'Sleeper (European)'; break;
			case 'XI': $category = 'International'; break;
			case 'XR': $category = 'Motorail'; break;
			case 'XU': $category = 'Unadvertised Express'; break;
			case 'XX': $category = 'Express Passenger'; break;
			case 'XZ': $category = 'Sleeper (Domestic)'; break;
			
			case 'BR': $category = 'Bus - Replacement due to engineering work'; break;
			case 'BS': $category = 'Bus - WTT service'; break;
			
			case 'EE': $category = 'Empty Coaching Stock'; break;
			case 'EL': $category = 'ECS, London Underground/Metro'; break;
			case 'ES': $category = 'ECS & Staff'; break;
			
			case 'JJ': $category = 'Postal'; break;
			case 'PM': $category = 'Post Office Controlled Parcels'; break;
			case 'PP': $category = 'Parcels'; break;
			case 'PV': $category = 'Empty NPCSS'; break;
			
			case 'DD': $category = 'Departmental'; break;
			case 'DH': $category = 'Civil Engineer'; break;
			case 'DI': $category = 'Mechanical & Electrical Engineer'; break;
			case 'DQ': $category = 'Stores'; break;
			case 'DT': $category = 'Test'; break;
			case 'DY': $category = 'Signal & Telecommunications Engineer'; break;
			
			case 'ZB': $category = 'Locomotive & Brake Van'; break;
			case 'ZZ': $category = 'Light Locomotive'; break;
			
			case 'J2': $category = 'RfD Automotive (Components)'; break;
			case 'H2': $category = 'RfD Automotive (Vehicles)'; break;
			case 'J3': $category = 'RfD Edible Products (UK Contracts)'; break;
			case 'J4': $category = 'RfD Industrial Materials (UK Contracts)'; break;
			case 'J5': $category = 'RfD Chemicals (UK Contracts)'; break;
			case 'J6': $category = 'RfD Building Materials (UK Contracts)'; break;
			case 'J8': $category = 'RfD General Merchandise (UK Contracts)'; break;
			case 'H8': $category = 'RfD European'; break;
			case 'J9': $category = 'RfD Freightliner (Contracts)'; break;
			case 'H9': $category = 'RfD Freightliner (Other)'; break;
			
			case 'A0': $category = 'Coal (Distributive)'; break;
			case 'E0': $category = 'Coal (Electricity) and MGR'; break;
			case 'B0': $category = 'Coal (Other) and Nuclear'; break;
			case 'B1': $category = 'Metals'; break;
			case 'B4': $category = 'Aggregates'; break;
			case 'B5': $category = 'Domestic & Industrial Waste'; break;
			case 'B6': $category = 'Building Materials'; break;
			case 'B7': $category = 'Petroleum Products'; break;
			
			case 'H0': $category = 'RfD European Channel Tunnel (Mixed Business)'; break;
			case 'H1': $category = 'RfD European Channel Tunnel Intermodal'; break;
			case 'H3': $category = 'RfD European Channel Tunnel Automotive'; break;
			case 'H4': $category = 'RfD European Channel Tunnel Contract Services'; break;
			case 'H5': $category = 'RfD European Channel Tunnel Haulmark'; break;
			case 'H6': $category = 'RfD European Channel Tunnel Joint Venture'; break;
			
			default: $category = 'Unknown'; break;
		}
		
		return $category;
	}
	
	public static function getServiceTrainClass($c) {
		switch(trim($c)) {
			case 'S': return 'Standard Class'; break;
			case 'B': default: return 'First & Standard Class'; break;
		}
	}
	
	public static function getServiceSleeperClass($c) {
		switch(trim($c)) {
			case 'S': return 'Standard Class'; break;
			case 'B': return 'First & Standard Class'; break;
			case 'F': return 'First Class'; break;
			default: return false; break;
		}
	}
	
	public static function getServiceReservations($r) {
		switch(trim($r)) {
			case 'A': return 'Seat Reservations Compulsory'; break;
			case 'E': return 'Reservations for Bicycles Essential'; break;
			case 'R': return 'Seat Reservations Recommended'; break;
			case 'S': return 'Seat Reservations Available'; break;
			default: return false; break;
		}
	}
	
	public static function getServiceOperatingCharacteristics($o) {
		$cat = array();
		
		for($i = 0; $i < strlen($o); $i++) {
			switch(trim(substr($o, $i, 1))) {
				case 'B': $cat[] = 'Vacuum Braked'; break;
				case 'C': $cat[] = 'Timed at 100mph'; break;
				case 'D': $cat[] = 'Driver Only Operated'; break;
				case 'E': $cat[] = 'Conveys Mk4 coaches'; break;
				case 'G': $cat[] = 'Trainman (Guard) required'; break;
				case 'M': $cat[] = 'Timed at 110mph'; break;
				case 'P': $cat[] = 'Push/Pull train'; break;
				case 'Q': $cat[] = 'Runs as Required'; break;
				case 'R': $cat[] = 'Air conditioned with PA system'; break;
				case 'S': $cat[] = 'Steam Heated'; break;
				case 'Y': $cat[] = 'Runs to Terminals/Guards as required'; break;
				case 'Z': $cat[] = 'May convey traffic to SB1C gauge. Not to be diverted from booked route without authority.'; break;
			}
		}
		
		if(count($cat) > 1) return $cat;
		elseif(count($cat)==1) return $cat[0];
		return false;
	}
	
	public static function getServiceCatering($c) {
		$cat = new stdClass;
		$cat->list = array();
		$cat->buffet = false;
		
		for($i = 0; $i < strlen($c); $i++) {
			switch(trim(substr($c, $i, 1))) {
				case 'C': $cat->buffet = true;$cat->list[] = 'Buffet'; break;
				case 'F': $cat->buffet = true;$cat->list[] = 'Restaurant Car available to First Class passengers'; break;
				case 'H': $cat->buffet = true;$cat->list[] = 'Hot Food service'; break;
				case 'M': $cat->buffet = true;$cat->list[] = 'Meal included for First Class passengers'; break;
				case 'P': $cat->list[] = 'Wheelchair only reservations'; break;
				case 'R': $cat->buffet = true;$cat->list[] = 'Restaurant'; break;
				case 'T': $cat->buffet = true;$cat->list[] = 'Trolley Service'; break;
			}
		}
		
		return $cat;
	}
	
	public static function getServiceActivity($a) {
		$cat = new stdClass;
		$cat->list = array();
		
		$cat->request = false;
		
		for($i = 0; $i < strlen($a); $i=$i+2) {
			switch(trim(substr($a, $i, 2))) {
				case 'A': $cat->list[] = 'Stops or shunts for other trains to pass'; break;
				case 'AE': $cat->list[] = 'Attach/detach assisting locomotive'; break;
				case 'BL': $cat->list[] = 'Stops for banking locomotive'; break;
				case 'C': $cat->list[] = 'Stops to change trainmen'; break;
				case 'D': $cat->list[] = 'Stops to set down passengers'; break;
				case '-D': $cat->list[] = 'Stops to detach vehicles'; break;
				case 'E': $cat->list[] = 'Stops for examination'; break;
				case 'G': $cat->list[] = 'National Rail Timetable data to add'; break;
				case 'H': $cat->list[] = 'Notional activity to prevent WTT timing columns merge'; break;
				case 'HH': $cat->list[] = 'Notional activity to prevent WTT timing columns merge, where a third column is involved'; break;
				case 'K': $cat->list[] = 'Passenger count point'; break;
				case 'KC': $cat->list[] = 'Ticket collection and examination point'; break;
				case 'KE': $cat->list[] = 'Ticket examination point'; break;
				case 'KF': $cat->list[] = 'Ticket Examination Point, 1st Class only'; break;
				case 'KS': $cat->list[] = 'Selective Ticket Examination Point'; break;
				case 'L': $cat->list[] = 'Stops to change locomotives'; break;
				case 'N': $cat->list[] = 'Stop not advertised'; break;
				case 'OP': $cat->list[] = 'Stops for other operating reasons'; break;
				case 'OR': $cat->list[] = 'Train Locomotive on rear'; break;
				case 'PR': $cat->list[] = 'Propelling between points shown'; break;
				case 'R': $cat->request = true; $cat->list[] = 'Stops when required'; break;
				case 'RM': $cat->list[] = 'Reversing movement, or driver changes ends'; break;
				case 'RR': $cat->list[] = 'Stops for locomotive to run round train'; break;
				case 'S': $cat->list[] = 'Stops for railway personnel only'; break;
				case 'T': $cat->list[] = 'Stops to take up and set down passengers'; break;
				case '-T': $cat->list[] = 'Stops to attach and detach vehicles'; break;
				case 'TB': $cat->list[] = 'Train begins (Origin)'; break;
				case 'TF': $cat->list[] = 'Train finishes (Destination)'; break;
				case 'TS': $cat->list[] = 'Detail Consist for TOPS Direct requested by EWS'; break;
				case 'TW': $cat->list[] = 'Stops (or at pass) for tablet, staff or token.'; break;
				case 'U': $cat->list[] = 'Stops to take up passengers'; break;
				case '-U': $cat->list[] = 'Stops to attach vehicles'; break;
				case 'W': $cat->list[] = 'Stops for watering of coaches'; break;
				case 'X': $cat->list[] = 'Passes another train at crossing point on single line'; break;
			}
		}
	
		return $cat;
	}
	
	public static function getServiceStpInd($s) {
		switch(trim($s)) {
			case 'P': return 'WTT'; break;
			case 'O': return 'VAR'; break;
			case 'N': return 'STP'; break;
			default: return ''; break;
		}
	}
	
	public function getLocationDetailFromCRS($crs) {
		$query = 'select tiploc, nalco, tps_description, stanox, crs, description from tiplocs where crs = "'. $this->escape($crs) .'" limit 0,1';
		
		if($s = $this->query($query)) {
			$b = $s->fetch_object();
			$b->tps_description = self::capitalizeWords(strtolower($b->tps_description));
		
			return $b;
		}
		else {
			return NULL;
		}
	}
	
	public function getLocationTableForTimes($tiploc, $date, $type, $public_only = true, $categories = NULL) {
		if($categories == NULL) $categories = self::getPassengerCategories();
		
		switch($type) {
			case 0: return $this->buildLocationTable($tiploc, $date, self::overlay, '0000', '0159H', $public_only, $categories);
			case 1: return $this->buildLocationTable($tiploc, $date, self::overlay, '0200', '0359H', $public_only, $categories);
			case 2: return $this->buildLocationTable($tiploc, $date, self::overlay, '0400', '0559H', $public_only, $categories);
			case 3: return $this->buildLocationTable($tiploc, $date, self::overlay, '0600', '0759H', $public_only, $categories);
			case 4: return $this->buildLocationTable($tiploc, $date, self::overlay, '0800', '0959H', $public_only, $categories);
			case 5: return $this->buildLocationTable($tiploc, $date, self::overlay, '1000', '1159H', $public_only, $categories);
			case 6: return $this->buildLocationTable($tiploc, $date, self::overlay, '1200', '1359H', $public_only, $categories);
			case 7: return $this->buildLocationTable($tiploc, $date, self::overlay, '1400', '1559H', $public_only, $categories);
			case 8: return $this->buildLocationTable($tiploc, $date, self::overlay, '1600', '1759H', $public_only, $categories);
			case 9: return $this->buildLocationTable($tiploc, $date, self::overlay, '1800', '1959H', $public_only, $categories);
			case 10: return $this->buildLocationTable($tiploc, $date, self::overlay, '2000', '2159H', $public_only, $categories);
			case 11: return $this->buildLocationTable($tiploc, $date, self::overlay, '2200', '2359H', $public_only, $categories);
		}
	}
	
	private function getScheduleDetails($train_uid, $date, $stp = NULL) {
		list($date, $day) = $this->getMySQLDate($date);
		
		if($stp == NULL)
			$query = 'select schedules.*, schedules_cache.*, otip.tps_description originname, dtip.tps_description destinationname from schedules left join schedules_cache on schedules.id = schedules_cache.id left join tiplocs as otip on otip.tiploc = schedules_cache.origin left join tiplocs dtip on dtip.tiploc = schedules_cache.destination where `train_uid` = "'. $this->escape($train_uid) .'" AND ("'. $date .'" between date_from and date_to) AND runs_'. $day .' = 1 AND (stp_indicator = "O" OR stp_indicator = "N") UNION select schedules.*, schedules_cache.*, otip.tps_description originname, dtip.tps_description destinationname from schedules left join schedules_cache on schedules.id = schedules_cache.id left join tiplocs as otip on otip.tiploc = schedules_cache.origin left join tiplocs dtip on dtip.tiploc = schedules_cache.destination left join schedules_stpcancel on schedules_stpcancel.id = schedules.id and ("'. $date .'" between cancel_from and cancel_to and cancel_'. $day .' = 1) where `train_uid` = "'. $this->escape($train_uid) .'" AND ("'. $date .'" between date_from and date_to) AND runs_'. $day .' = 1 AND stp_indicator = "P" AND schedules_stpcancel.id is null';
		else
			$query = 'select schedules.*, schedules_cache.*, otip.tps_description originname, dtip.tps_description destinationname from schedules left join schedules_cache on schedules.id = schedules_cache.id left join tiplocs as otip on otip.tiploc = schedules_cache.origin left join tiplocs dtip on dtip.tiploc = schedules_cache.destination where `train_uid` = "'. $this->escape($train_uid) .'" AND ("'. $date .'" between date_from and date_to) AND runs_'. $day .' = 1 AND stp_indicator = "'. $this->escape($stp) .'"';
	
		if($s = $this->query($query)) {			
			return $s->fetch_object();
		}
		
		return NULL;
	}
	
	private function generateCategoryQuery($categories, $scheds = false) {
		if($categories === NULL) return '';
		
		$categoryText = '';
		$statusText = '';
		
		$catArray = $categories->categories;
		$staArray = $categories->status;
		
		if(is_array($catArray) && count($catArray) > 0) {
			$categoryText .= (($scheds) ? 'scheds': 'schedules').'.category in ';
			
			foreach($catArray as &$cat) {
				$cat = '"'. $this->escape($cat) .'"';
			}
			
			$categoryText .= '('. implode(',', $catArray) .') ';
		}
		
		if(is_array($staArray) && count($staArray) > 0) {
			$statusText .= (($scheds) ? 'scheds': 'schedules').'.status in ';
			
			foreach($staArray as &$sta) {
				$sta = '"'. $this->escape($sta) .'"';
			}
			
			$statusText .= '('. implode(',', $staArray) .') ';
		}
		
		if($categoryText != "" && $statusText != "") {
			return ' AND ('. $categoryText .' AND '. $statusText .') ';
		}
		else if($categoryText != "" && $statusText == "") {
			return ' AND '. $categoryText;
		}
		else if($categoryText == "" && $statusText != "") {
			return ' AND '. $statusText;
		}
		else {
			return '';
		}
	}
	
	private function buildLocationTable($tiploc, $date, $stp = true, $start = '0000', $end = '2359H', $public_only = true, $categories = NULL) {	
		// get the proper date, and escape the tiploc
		list($dateCurr, $dayCurr) = $this->getMySQLDate($date);
		list($datePrev, $dayPrev) = $this->getMySQLDate($date .' -1 day');
		$tiploc=$this->escape($tiploc);
		$services = array();
		
		$category = $this->generateCategoryQuery($categories);
		
		$ordertime = 'IF(locations.pass="" AND locations.departure="", locations.arrival, IF(locations.pass="", locations.departure, locations.pass)) as ordertime, ';
		$wherePublic = ' ';
		
		if($public_only) {
			$ordertime .= ' IF(locations.public_departure = "" AND locations.public_arrival = "", NULL, IF(locations.public_departure="", locations.public_arrival, locations.public_departure)) as publictime'; 
			$wherePublic = ' AND publictime IS NOT NULL';
		}
		
		$selectCurr = 'schedules.*, locations.location_type, locations.tiploc_code, locations.tiploc_instance, locations.arrival, locations.public_arrival, locations.pass, locations.departure, locations.public_departure, locations.platform, locations.line, locations.path, locations.engineering_allowance, locations.pathing_allowance, locations.performance_allowance, locations.activity, '. $ordertime .', schedules_cache.origin, schedules_cache.origin_time, schedules_cache.destination, schedules_cache.destination_time, "'. $dateCurr .'" as dep_date, origintiplocs.tps_description as origin_desc, destintiplocs.tps_description as destination_desc FROM timetables.locations as locations JOIN timetables.schedules as schedules ON schedules.id = locations.id LEFT JOIN timetables.schedules_cache as schedules_cache ON schedules_cache.id = schedules.id LEFT JOIN locations_alternatives ola ON ola.additional_tiploc = schedules_cache.origin LEFT JOIN locations_alternatives dla ON dla.additional_tiploc = schedules_cache.destination LEFT JOIN tiplocs as origintiplocs ON IF(ola.tiploc IS NULL, schedules_cache.origin, ola.tiploc) = origintiplocs.tiploc LEFT JOIN tiplocs as destintiplocs ON IF(dla.tiploc IS NULL, schedules_cache.destination, dla.tiploc) = destintiplocs.tiploc';
		$selectPrev = 'schedules.*, locations.location_type, locations.tiploc_code, locations.tiploc_instance, locations.arrival, locations.public_arrival, locations.pass, locations.departure, locations.public_departure, locations.platform, locations.line, locations.path, locations.engineering_allowance, locations.pathing_allowance, locations.performance_allowance, locations.activity, '. $ordertime .', schedules_cache.origin, schedules_cache.origin_time, schedules_cache.destination, schedules_cache.destination_time, "'. $datePrev .'" as dep_date, origintiplocs.tps_description as origin_desc, destintiplocs.tps_description as destination_desc FROM timetables.locations as locations JOIN timetables.schedules as schedules ON schedules.id = locations.id LEFT JOIN timetables.schedules_cache as schedules_cache ON schedules_cache.id = schedules.id LEFT JOIN locations_alternatives ola ON ola.additional_tiploc = schedules_cache.origin LEFT JOIN locations_alternatives dla ON dla.additional_tiploc = schedules_cache.destination LEFT JOIN tiplocs as origintiplocs ON IF(ola.tiploc IS NULL, schedules_cache.origin, ola.tiploc) = origintiplocs.tiploc LEFT JOIN tiplocs as destintiplocs ON IF(dla.tiploc IS NULL, schedules_cache.destination, dla.tiploc) = destintiplocs.tiploc';
				
		$destination = $this->getAssociatedCodesForLocations($tiploc);
		
		if(is_array($destination)) {
			foreach($destination as &$loc) {
				$loc = '"'. $this->escape($loc) .'"';
			}
			
			$locQuery .= 'IN ('. implode(',', $destination) .')';
		}
		else {
			$locQuery .= '= "'. $this->escape($destination) .'"';
		}
		
		$whereCurr = 'locations.tiploc_code '. $locQuery. ' AND ("'. $dateCurr .'" BETWEEN schedules.date_from AND schedules.date_to) AND schedules.runs_'. $dayCurr .' = 1 '. $category;
		$wherePrev = 'locations.tiploc_code '. $locQuery .' AND ("'. $datePrev .'" BETWEEN schedules.date_from AND schedules.date_to) AND schedules.runs_'. $dayPrev .' = 1 '. $category;
		
		if($start != '0000' || $end != '2359H') {
			$havingCurr = 'HAVING ordertime >= origin_time AND (ordertime BETWEEN "'. $start .'" AND "'. $end .'") '. $wherePublic;
			$havingPrev = 'HAVING ordertime < origin_time AND (ordertime BETWEEN "'. $start .'" AND "'. $end .'") '. $wherePublic;
		}
		else {
			$havingCurr = 'HAVING ordertime >= origin_time'. $wherePublic;
			$havingPrev = 'HAVING ordertime < origin_time'. $wherePublic;
		}
		
		$services = array();
		
		if($stp == true) {			
			$query = 'SELECT * FROM (SELECT '. $selectCurr .' WHERE '. $whereCurr .' AND (schedules.stp_indicator = "N" OR schedules.stp_indicator = "O") '. $havingCurr .' UNION SELECT '. $selectCurr .' LEFT JOIN schedules as schedoverlay ON (schedoverlay.stp_indicator = "O" AND schedoverlay.train_uid = schedules.train_uid AND ("'. $dateCurr .'" BETWEEN schedoverlay.date_from AND schedoverlay.date_to) AND schedoverlay.runs_'. $dayCurr .' = 1) LEFT JOIN schedules_stpcancel ON (schedules.id = schedules_stpcancel.id AND ("'. $dateCurr .'" BETWEEN schedules_stpcancel.cancel_from AND schedules_stpcancel.cancel_to) AND cancel_'. $dayCurr .' = 1) WHERE '. $whereCurr .' AND (schedules.stp_indicator = "N" OR schedules.stp_indicator = "P") AND schedoverlay.train_uid IS NULL AND schedules_stpcancel.id is null '. $havingCurr .' UNION 
			
			SELECT '. $selectPrev .' WHERE '. $wherePrev .' AND (schedules.stp_indicator = "N" OR schedules.stp_indicator = "O") '. $havingPrev .' UNION SELECT '. $selectPrev .' LEFT JOIN schedules as schedoverlay ON (schedoverlay.stp_indicator = "O" AND schedoverlay.train_uid = schedules.train_uid AND ("'. $datePrev .'" BETWEEN schedoverlay.date_from AND schedoverlay.date_to) AND schedoverlay.runs_'. $dayPrev .' = 1)  LEFT JOIN schedules_stpcancel ON (schedules.id = schedules_stpcancel.id AND ("'. $datePrev .'" BETWEEN schedules_stpcancel.cancel_from AND schedules_stpcancel.cancel_to) AND cancel_'. $dayPrev .' = 1) WHERE '. $wherePrev .' AND (schedules.stp_indicator = "N" OR schedules.stp_indicator = "P") AND schedoverlay.train_uid IS NULL AND schedules_stpcancel.id is null '. $havingPrev .') as tt ORDER BY '. (($public_only) ? 'publictime' : 'ordertime') .' ASC';
																														
			if($q = $this->query($query)) {
				
				while($row = $q->fetch_object()) {
					$row->origin_desc = self::capitalizeWords($row->origin_desc);
					$row->destination_desc = self::capitalizeWords($row->destination_desc);
				
					$services[] = $row;
				}
				$q->free();	
			}
		}	
		
		return $services;
	}
	
	private static function capitalizeWords($words, $charList = " ()&.") {
		$words = strtolower($words);
	
	    // Use ucwords if no delimiters are given
	    if (!isset($charList)) {
	        return ucwords($words);
	    }
	    
	    // Go through all characters
	    $capitalizeNext = true;
	    
	    for ($i = 0, $max = strlen($words); $i < $max; $i++) {
	        if (strpos($charList, $words[$i]) !== false) {
	            $capitalizeNext = true;
	        } else if ($capitalizeNext) {
	            $capitalizeNext = false;
	            $words[$i] = strtoupper($words[$i]);
	        }
	    }
	    
	    return $words;
	}
	
	public function getService($uid, $date, $publicOnly = true, $categories = NULL, $fromPoint = "", $toPoint = "") {
		list($dt, $day) = $this->getMySQLDate($date);		
		$category = $this->generateCategoryQuery($categories);
	
		if($d = $this->query('SELECT schedules.*, schedules_cache.origin, schedules_cache.public_origin, origintiploc.tps_description origin_description, schedules_cache.destination, destinationtiploc.tps_description destination_description, schedules_cache.public_destination FROM schedules LEFT JOIN schedules_stpcancel ON (schedules.id = schedules_stpcancel.id AND ("'. $dt .'" BETWEEN schedules_stpcancel.cancel_from AND schedules_stpcancel.cancel_to) AND cancel_'. $day .' = 1) LEFT JOIN schedules_cache ON schedules.id = schedules_cache.id LEFT JOIN locations_alternatives ola ON schedules_cache.origin = ola.additional_tiploc LEFT JOIN tiplocs origintiploc ON IF(ola.tiploc IS NULL, origin, ola.tiploc) = origintiploc.tiploc LEFT JOIN locations_alternatives dla ON destination = dla.additional_tiploc LEFT JOIN tiplocs destinationtiploc ON IF(dla.tiploc IS NULL, destination, dla.tiploc) = destinationtiploc.tiploc WHERE train_uid = "'. $this->escape($uid) .'" AND ("'. $dt .'" BETWEEN schedules.date_from AND schedules.date_to) AND runs_'. $day .'=1 AND schedules_stpcancel.id is null '. $category .' ORDER BY schedules.stp_indicator DESC LIMIT 0,1')) {
			$schedule = $d->fetch_object();
			
			$fromLocOrder = false;
			$toLocOrder = false;
			
			if(is_array($fromPoint)) {
				foreach($fromPoint as &$pt) {
					$pt = '"'. $this->escape($pt) .'"';
				}
				unset($pt);
			
				if($lp = $this->query('SELECT location_order FROM locations WHERE id = "'. $schedule->id .'" AND tiploc_code IN ('. implode(',', $fromPoint) .') ORDER BY location_order ASC LIMIT 0,1')){
					$r = $lp->fetch_object();
					
					$fromLocOrder = $r->location_order;
				}
			}
			else if($fromPoint != "") {
				if($lp = $this->query('SELECT location_order FROM locations WHERE id = "'. $schedule->id .'" AND tiploc_code = "'. $this->escape($fromPoint) .'" ORDER BY location_order ASC LIMIT 0,1')){
					$r = $lp->fetch_object();
					
					$fromLocOrder = $r->location_order;
				}
			}
			
			if(is_array($toPoint)) {
				foreach($toPoint as &$pt) {
					$pt = '"'. $this->escape($pt) .'"';
				}
				unset($pt);
			
				if($lp = $this->query('SELECT location_order FROM locations WHERE id = "'. $schedule->id .'" AND tiploc_code IN ('. implode(',', $toPoint) .') ORDER BY location_order ASC LIMIT 0,1')){
					$r = $lp->fetch_object();
					
					$toLocOrder = $r->location_order;
				}
			}
			else if($toPoint != "") {
				if($lp = $this->query('SELECT location_order FROM locations WHERE id = "'. $schedule->id .'" AND tiploc_code = "'. $this->escape($toPoint) .'" ORDER BY location_order ASC LIMIT 0,1')){
					$r = $lp->fetch_object();
					
					$toLocOrder = $r->location_order;
				}
			}
			
			
			// get the location data
			$locs = array();
			if($l = $this->query('SELECT locations.*, tiplocs.tps_description as `tps_description`, tiplocs.crs as `crs` FROM locations LEFT JOIN locations_alternatives la ON tiploc_code = la.additional_tiploc LEFT JOIN tiplocs ON IF(la.tiploc IS NULL, tiploc_code, la.tiploc) = tiplocs.tiploc WHERE id = "'. $schedule->id .'" '. (($fromLocOrder !== false) ? ' AND location_order > '. $fromLocOrder.' ' : '') . (($toLocOrder !== false) ? ' AND location_order <= '. $toLocOrder .' ' : '') . (($publicOnly) ? ' AND (locations.public_arrival != "" OR locations.public_departure != "")' : ''). ' ORDER BY location_order ASC')) {
				while($row = $l->fetch_object()) {
					$locs[] = $row;
				}
			}
			unset($l);
			
			// find any location_change records
			$changeRecords = false;
			$chng = array();
			if($c = $this->query('SELECT locations_change.* FROM locations_change WHERE id = "'. $schedule->id .'"')) {
				while($row = $c->fetch_object()) { $chng[] = $row; $changeRecords = true; }
			}
			unset($c);
						
			// now find associations
			$assoc = array();
			
			if($a = $this->query('select associations.* from associations where (`main_train_uid` = "'. $schedule->train_uid .'" or `assoc_train_uid` = "'. $schedule->train_uid .'") AND ("'. $dt .'" between date_from and date_to) AND runs_'. $day .' = 1 AND (stp_indicator = "O" OR stp_indicator = "N") UNION select associations.* from associations left join associations assocoverlay on (assocoverlay.stp_indicator = "O" AND assocoverlay.main_train_uid = associations.main_train_uid AND assocoverlay.assoc_train_uid = associations.assoc_train_uid  AND ("'. $dt .'" between assocoverlay.date_from and assocoverlay.date_to) AND assocoverlay.runs_'. $day .' = 1) left join associations_stpcancel on associations_stpcancel.id = associations.id and ("'. $dt .'" between cancel_from and cancel_to and cancel_'. $day .' = 1) where (associations.`main_train_uid` = "'. $schedule->train_uid .'" or associations.`assoc_train_uid` = "'. $schedule->train_uid .'") AND ("'. $dt .'" between associations.date_from and associations.date_to) AND associations.runs_'. $day .' = 1 AND associations.stp_indicator = "P" AND associations_stpcancel.id is null and assocoverlay.main_train_uid is null')) {
				while($row = $a->fetch_object()) $assoc[] = $row;
			}
			unset($a);
						
			// now processâ€¦
			$service = new stdClass;
			$service->id = $schedule->id;
			$service->unique_id = $schedule->unique_id;
			$service->train_uid = $schedule->train_uid;
			$service->date_from = $schedule->date_from;
			$service->date_to = $schedule->date_to;
			$service->operates = self::generateDaysRun($schedule->runs_mo, $schedule->runs_tu, $schedule->runs_we, $schedule->runs_th, $schedule->runs_fr, $schedule->runs_sa, $schedule->runs_su);
			$service->status = self::getServiceStatus($schedule->status);
			$service->statusCode = $schedule->status;
			$service->category = self::getServiceCategory($schedule->category);
			$service->categoryCode = $schedule->category;
			$service->train_id = $schedule->train_identity;
			$service->headcode = $schedule->headcode;
			$service->service_code = $schedule->service_code;
			$service->portion_id = $schedule->portion_id;
			$service->power = $schedule->power_type;
			$service->timing_load = $schedule->timing_load;
			$service->speed = $schedule->speed;
			$service->operatingCharacteristics = self::getServiceOperatingCharacteristics($schedule->operating_characteristics);
			$service->operatingCharacteristicsCode = $schedule->operating_characteristics;
			$service->trainClass = self::getServiceTrainClass($schedule->train_class);
			$service->trainClassCode = $schedule->train_class;
			$service->sleeperClass = self::getServiceSleeperClass($schedule->sleepers);
			$service->sleeperClassCode = $schedule->sleepers;
			$service->reservations = self::getServiceReservations($schedule->reservations);
			$service->reservationsCode = $schedule->reservations;
			$service->catering = self::getServiceCatering($schedule->catering_code);
			$service->cateringCode = $schedule->catering_code;
			$service->stp = self::getServiceStpInd($schedule->stp_indicator);
			$service->stpIndicator = $schedule->stp_indicator;
			$service->atoc = $schedule->atoc_code;
			$service->ats = $schedule->ats_code;
			
			// we can generate an rsid if headcode is 4 chars
			if(strlen(trim($schedule->headcode)) == 4) {
				$service->rsid = $service->atoc . trim($service->headcode);
			} else $service->rsid = '';
			
			unset($schedule);
			
			$noOfDays = 0;
			
			$service->locations = array();
			foreach($locs as $loc) {
				$location = new stdClass;
				$location->type = $loc->location_type;
				$location->tiploc = $loc->tiploc_code;
				$location->tiplocInstance = $loc->tiploc_instance;
				$location->crs = $loc->crs;
				$location->name = $loc->tps_description;
				$location->correctedName = self::capitalizeWords(strtolower($loc->tps_description));
				$location->publicCall = ($loc->public_arrival != "" || $loc->public_departure != "") ? true: false;
				$location->arrival = $loc->arrival;
				$location->publicArrival = $loc->public_arrival;
				$location->pass = $loc->pass;
				$location->departure = $loc->departure;
				$location->publicDeparture = $loc->public_departure;
				$location->platform = $loc->platform;
				$location->line = $loc->line;
				$location->path = $loc->path;
				$location->engA = $loc->engineering_allowance;
				$location->pthA = $loc->pathing_allowance;
				$location->prfA = $loc->performance_allowance;
				$location->act = self::getServiceActivity($loc->activity);
				$location->actCode = $loc->activity;
				$location->assoc = array();
				
				if($publicOnly) {
					if($location->publicDeparture != "") { $ordertime = $location->publicDeparture; }
					else { $ordertime = $location->publicArrival; }
				}
				else {
					if($location->departure != "") { $ordertime = $location->departure; }
					elseif($location->pass != "") { $ordertime = $location->pass; }
					else { $ordertime = $location->arrival; }
				}

				if(isset($prevtime)) {
					if($ordertime < $prevtime) $noOfDays++;
				}
				$prevtime = $ordertime;
				$location->date = self::getMySQLDateYMD($dt .' +'. $noOfDays .' days');
				$location->period = self::getPeriodForTime($ordertime);
				
				foreach($assoc as $association) {
					if($association->location == $location->tiploc) {
						if($service->train_uid == $association->main_train_uid) {
							if($loc->tiploc_instance != $association->base_location_suffix) continue;
							
							$a = new stdClass;
							$a->main = true;
							$a->associate = $association->assoc_train_uid;
							$a->category = $association->category;
							if($association->date_indicator == "N") 
								$a->date = self::getMySQLDateYMD($dt .' +1 day');
							elseif($association->date_indicator == "P")
								$a->date = self::getMySQLDateYMD($dt .' -1 day');
							else
								$a->date = $dt;
							
							$a->public = ($association->assoc_type == "P") ? true : false;
							$associationTrain = ($this->getScheduleDetails($a->associate, $a->date));
							$a->associateUUID = $associationTrain->unique_id;
							$a->associateTrainUid = $associationTrain->train_uid;
							$a->associateTrainId = $associationTrain->train_identity;
							$a->associationOrigin = self::capitalizeWords($associationTrain->originname);
							$a->associationOriginCode = $associationTrain->origin;
							$a->associationDestination = self::capitalizeWords($associationTrain->destinationname);
							$a->associationDestinationCode = $associationTrain->destination;
							
							$location->assoc[] = $a;
						} 
						elseif($service->train_uid == $association->assoc_train_uid){
							if($loc->tiploc_instance != $association->assoc_location_suffix) continue;
							
							$a = new stdClass;
							$a->main = false;
							$a->associate = $association->main_train_uid;
							$a->category = $association->category;
							if($association->date_indicator == "N") 
								$a->date = self::getMySQLDateYMD($dt .' -1 day');
							elseif($association->date_indicator == "P")
								$a->date = self::getMySQLDateYMD($dt .' +1 day');
							else
								$a->date = $dt;
							
							//print $a->date .' '.$a->associate; 
							$a->public = ($association->assoc_type == "P") ? true : false;
							$associationTrain = ($this->getScheduleDetails($a->associate, $a->date));
							$a->associateUUID = $associationTrain->unique_id;
							$a->associateTrainUid = $associationTrain->train_uid;
							$a->associateTrainId = $associationTrain->train_identity;
							$a->associationOrigin = self::capitalizeWords($associationTrain->originname);
							$a->associationOriginCode = $associationTrain->origin;
							$a->associationDestination = self::capitalizeWords($associationTrain->destinationname);
							$a->associationDestinationCode = $associationTrain->destination;
							
							$location->assoc[] = $a;
						}
					}
				}
				
				if(count($location->assoc)==0) $location->assoc = false;
				
				$service->locations[] = $location;
			}
			
			return $service;
		}
		
		return false;
	}
	
	private function buildLocationOriginQuery($origin, $destination, $date, $category = NULL) {
		list($dt, $day) = $this->getMySQLDate($date);
		
		$query = 'select origin.public_departure deptime, origin.platform origin_platform, destination.public_arrival arrtime, destination.platform destination_platform, schedules_cache.origin, serviceorigin.tps_description origin_description, serviceorigin.crs origin_crs, schedules_cache.origin_time, schedules_cache.public_origin,  schedules_cache.destination, servicedestination.tps_description destination_description, servicedestination.crs destination_crs, schedules_cache.destination_time, schedules_cache.public_destination, schedules.unique_id, schedules.train_identity, "'. $dt .'" as date, schedules.train_uid, schedules.atoc_code atoc, schedules.sleepers, schedules.catering_code from locations origin join locations destination on (origin.id = destination.id and destination.tiploc_code ';
		
		if(is_array($destination)) {
			foreach($destination as &$loc) {
				$loc = '"'. $this->escape($loc) .'"';
			}
			
			$query .= 'IN ('. implode(',', $destination) .')';
		}
		else {
			$query .= '= "'. $this->escape($destination) .'"';
		}
		
		$query .= ' and destination.location_order > origin.location_order) left join schedules_cache on origin.id = schedules_cache.id LEFT JOIN locations_alternatives ola ON ola.additional_tiploc = schedules_cache.origin LEFT JOIN locations_alternatives dla ON dla.additional_tiploc = schedules_cache.destination left join tiplocs serviceorigin on IF(ola.tiploc IS NULL, schedules_cache.origin, ola.tiploc) = serviceorigin.tiploc left join tiplocs servicedestination on IF(dla.tiploc IS NULL, schedules_cache.destination, dla.tiploc) = servicedestination.tiploc left join schedules on origin.id = schedules.id left join schedules schedules_stp on (schedules.train_uid = schedules_stp.train_uid and ("'. $dt .'" between schedules_stp.date_from and schedules_stp.date_to) and schedules_stp.runs_'. $day .' = 1 and schedules_stp.stp_indicator = "O") left join schedules_stpcancel on (origin.id = schedules_stpcancel.id AND ("'. $dt .'" between schedules_stpcancel.cancel_from and schedules_stpcancel.cancel_to) AND schedules_stpcancel.cancel_'. $day .'=1) where origin.tiploc_code ';
		
		if(is_array($origin)) {
			foreach($origin as &$loc) {
				$loc = '"'. $this->escape($loc) .'"';
			}
			
			$query .= 'IN ('. implode(',', $origin) .')';
		} 
		else {
			$query .= '= "'. $this->escape($origin) .'"';
		}
		
		$query .= ' and origin.public_departure != "" and destination.public_arrival != "" and ("'. $dt .'" between schedules.date_from and schedules.date_to) and schedules.runs_'. $day .' = 1 and (schedules.stp_indicator = "N" or schedules.stp_indicator = "O" or (schedules.stp_indicator = "P" AND schedules_stp.train_uid is null AND schedules_stpcancel.id is null))';
		
		$query .= $this->generateCategoryQuery($category);
		
		return $query;
	}
	
	private function buildAssociativeLocationQuery($origin, $destination, $date, $category = NULL) {
		list($dt, $day) = $this->getMySQLDate($date);
		
		$query = 'select origin.public_departure deptime, origin.platform origin_platform, destination.public_arrival arrtime, destination.platform destination_platform, scheds_cache.origin, serviceorigin.tps_description origin_description, serviceorigin.crs origin_crs, scheds_cache.origin_time, scheds_cache.public_origin,  scheds_cache.destination, servicedestination.tps_description destination_description, servicedestination.crs destination_crs, scheds_cache.destination_time, scheds_cache.public_destination, scheds.display_uid train_uid, "" train_identity, "'. $dt .'" as date, scheds.display_uid unique_id, scheds.atoc_code atoc, scheds.sleepers, scheds.catering_code from associations_locations origin join associations_locations destination on (origin.id = destination.id and destination.tiploc_code ';
		
		if(is_array($destination)) {
			foreach($destination as &$loc) {
				$loc = '"'. $this->escape($loc) .'"';
			}
			
			$query .= 'IN ('. implode(',', $destination) .')';
		}
		else {
			$query .= '= "'. $this->escape($destination) .'"';
		}
		
		$query .= ' and destination.location_order > origin.location_order) left join associations_schedules_cache as scheds_cache on origin.id = scheds_cache.id LEFT JOIN locations_alternatives ola ON ola.additional_tiploc = scheds_cache.origin LEFT JOIN locations_alternatives dla ON dla.additional_tiploc = scheds_cache.destination left join tiplocs serviceorigin on IF(ola.tiploc IS NULL, scheds_cache.origin, ola.tiploc) = serviceorigin.tiploc left join tiplocs servicedestination on IF(dla.tiploc IS NULL, scheds_cache.destination, dla.tiploc) = servicedestination.tiploc left join associations_schedules as scheds on origin.id = scheds.id left join associations_schedules scheds_stp on (scheds.actual_uid = scheds_stp.actual_uid and ("'. $dt .'" between scheds_stp.date_from and scheds_stp.date_to) and scheds_stp.runs_'. $day .' = 1 and scheds_stp.stp_indicator = "O") left join associations_schedules_stpcancel scheds_stpcancel on (origin.id = scheds_stpcancel.id AND ("'. $dt .'" between scheds_stpcancel.cancel_from and scheds_stpcancel.cancel_to) AND scheds_stpcancel.cancel_'. $day .'=1) where origin.tiploc_code ';
		
		if(is_array($origin)) {
			foreach($origin as &$loc) {
				$loc = '"'. $this->escape($loc) .'"';
			}
			
			$query .= 'IN ('. implode(',', $origin) .')';
		} 
		else {
			$query .= '= "'. $this->escape($origin) .'"';
		}
		
		$query .= ' and origin.public_departure != "" and destination.public_arrival != "" and ("'. $dt .'" between scheds.date_from and scheds.date_to) and scheds.runs_'. $day .' = 1 and (scheds.stp_indicator = "N" or scheds.stp_indicator = "O" or (scheds.stp_indicator = "P" AND scheds_stp.actual_uid is null AND scheds_stpcancel.id is null))';
		
		$query .= $this->generateCategoryQuery($category, true);
		
		return $query;
	}
	
	public function findTrainsBetweenLocations($o, $d, $date, $time = false, $public = true, $category = NULL) {
		list($dateCurr, $dayCurr) = $this->getMySQLDate($date);
		list($datePrev, $dayPrev) = $this->getMySQLDate($date .' -1 day');
		
		$origin = $this->getAssociatedCodesForLocations($o);
		$destination = $this->getAssociatedCodesForLocations($d);
				
		$startQuery = '';
		$endQuery = '';
		
		if($time !== false) {
			$time = DateTime::createFromFormat('Y-m-d Hi', $dateCurr .' '. $time);
						
			// start time
			$startTime = clone $time;
			$startTime->sub(new DateInterval('PT45M'));
						
			// end time
			$endTime = clone $time;
			$endTime->add(new DateInterval('PT60M'));
			$endTimeInt = intval($time->format('Hi')) + 100;
			if(intval($startTime->format('Hi')) > intval($endTime->format('Hi'))) $endTimeInt += 2400;
			
			$endTime->add(new DateInterval('PT30M'));
						
			if((int)$startTime->format('Ymd') < (int)$time->format('Ymd')) {
				$startQuery = 'SELECT * FROM (SELECT * FROM ('. $this->buildLocationOriginQuery($origin, $destination, $startTime->format('Y-m-d'), $category) .' AND origin.departure >= origin_time UNION '. $this->buildAssociativeLocationQuery($origin, $destination, $startTime->format('Y-m-d'), $category) .' AND origin.departure >= origin_time UNION '. $this->buildLocationOriginQuery($origin, $destination, $startTime->sub(new DateInterval('P1D'))->format('Y-m-d'), $category) .' AND origin.departure < origin_time UNION '. $this->buildAssociativeLocationQuery($origin, $destination, $startTime->sub(new DateInterval('P1D'))->format('Y-m-d'), $category) .' AND origin.departure < origin_time) t1 WHERE (t1.deptime BETWEEN "'. $startTime->format('Hi') .'" AND "2359H") ORDER BY t1.deptime ASC, t1.arrtime DESC) t7 UNION ';
				$startTime = "0000";
			}
			else {
				$startTime = $startTime->format('Hi');
			}
			
			if((int)$endTime->format('Ymd') > (int)$time->format('Ymd')) {
				$endQuery = ' UNION SELECT * FROM (SELECT * FROM ('. $this->buildLocationOriginQuery($origin, $destination, $endTime->format('Y-m-d'), $category) .' AND origin.departure >= origin_time UNION '. $this->buildAssociativeLocationQuery($origin, $destination, $endTime->format('Y-m-d'), $category) .' AND origin.departure >= origin_time UNION '. $this->buildLocationOriginQuery($origin, $destination, $endTime->sub(new DateInterval('P1D'))->format('Y-m-d'), $category) .' AND origin.departure < origin_time UNION '. $this->buildAssociativeLocationQuery($origin, $destination, $endTime->sub(new DateInterval('P1D'))->format('Y-m-d'), $category) .' AND origin.departure < origin_time) t2 WHERE (t2.deptime BETWEEN "0000" AND "'. $endTime->format('Hi') .'") ORDER BY t2.deptime ASC, t2.arrtime DESC) t6';
				$endTime = "2359H";
			}
			else {
				$endTime = $endTime->format('Hi');
			}
		}
		
		$query = $startQuery .' SELECT * FROM (SELECT * FROM ('. $this->buildLocationOriginQuery($origin, $destination, $dateCurr, $category) .' AND origin.departure >= origin_time UNION '. $this->buildAssociativeLocationQuery($origin, $destination, $dateCurr, $category) .' AND origin.departure >= origin_time UNION '. $this->buildLocationOriginQuery($origin, $destination, $datePrev, $category) .' AND origin.departure < origin_time UNION '. $this->buildAssociativeLocationQuery($origin, $destination, $datePrev, $category) .' AND origin.departure < origin_time) t '. (($time !== false) ? 'WHERE (t.deptime BETWEEN "'. $startTime .'" AND "'. $endTime .'")' : '') .' ORDER BY t.deptime ASC,t.arrtime DESC) t5 '. $endQuery;
				
		if($q = $this->query($query)) {
			$services = array();
			$startTime = NULL;
			
			while($service = $q->fetch_object()) {
				if($startTime == NULL) {
					$startTime = intval($service->deptime);
				}
			
				$service->origin_description = self::capitalizeWords(strtolower($service->origin_description));
				$service->destination_description = self::capitalizeWords(strtolower($service->destination_description));
				$service->sleeper = ($service->sleepers != "") ? true : false;
				$service->catering = $this->getServiceCatering($service->catering_code);
				
				// process the datesâ€¦
				$depdate = DateTime::createFromFormat('Y-m-d Hi', $service->date . ' ' . $service->deptime);
				$service->depdate = ($service->deptime < $service->public_origin) ? $depdate->add(new DateInterval('P1D'))->format('d/m/Y') : $depdate->format('d/m/Y');
				
				$arrdate = DateTime::createFromFormat('Y-m-d Hi', $service->date . ' ' . $service->arrtime);
				$service->arrdate = ($service->arrtime < $service->public_origin) ? $arrdate->add(new DateInterval('P1D'))->format('d/m/Y') : $arrdate->format('d/m/Y');
				
				$duration = $depdate->diff($arrdate, true);
				$service->duration = intval((intval($duration->format("%h"))*60) + intval($duration->format("%i")));
				
				$intDep = intval($service->deptime);
				$intArr = intval($service->arrtime);
			
				$service->depCal = ($intDep < $startTime) ? $intDep+2400 : $intDep;
				$service->arrCal = ($intArr < $startTime) ? $intArr+2400 : $intArr;
				$service->displayType = 1;
				$startTime = $service->depCal;
				
				// NULL - do not display
				// 1 - fastest
				// 2 - slower
				
				$services[] = $service;
			}
			
			for($i = 0; $i < count($services); $i++) {
				if($services[$i]->depCal > $endTimeInt && $time !== false)
					$services[$i]->displayType = NULL;
				elseif($i+1 < count($services) && $services[$i+1]->arrCal < $services[$i]->arrCal) 
					$services[$i]->displayType = 2;
				elseif($i+2 < count($services) && $services[$i+2]->arrCal < $services[$i]->arrCal) 
					$services[$i]->displayType = 2;
			}
			
			return (count($services) > 0) ? $services: false;
		}
		return NULL;
	}
	
	private function getAssociatedCodesForLocations($loc) {		
		if($r =  $this->query('SELECT additional_tiploc FROM locations_alternatives WHERE tiploc = "'. $this->escape($loc) .'"')) {
			$results = array();
			$results[] = trim($loc);
			
			while($row = $r->fetch_object())
				$results[] = $row->additional_tiploc;
				
			if(count($results) > 1) return $results;
			elseif(count($results) == 1) return $results[0];
		}
		
		return $loc;
	}
	
	private function invertAssociatedLocation($loc) {
		if($r =  $this->query('SELECT tiploc FROM locations_alternatives WHERE additional_tiploc = "'. $this->escape($loc) .'"')) {
			$results = array();
			$results[] = trim($loc);
			
			while($row = $r->fetch_object())
				$results[] = $row->tiploc;
				
			if(count($results) > 1) return $results;
			elseif(count($results) == 1) return $results[0];
		}
		
		return $loc;
	}
	
	public function getCRSFromLocation($loc) {
		$loc = $this->escape($loc);
		$query = 'SELECT * FROM (SELECT actual_tiploc.crs FROM tiplocs JOIN locations_alternatives ON tiplocs.tiploc = locations_alternatives.`additional_tiploc` JOIN tiplocs actual_tiploc ON locations_alternatives.tiploc = actual_tiploc.tiploc WHERE tiplocs.crs != "" AND (tiplocs.crs = "'. $loc .'" OR tiplocs.tps_description = "'. $loc .'") UNION SELECT crs FROM tiplocs WHERE crs != "" AND (crs = "'. $loc .'" OR tps_description = "'. $loc .'")) t LIMIT 0,1';
		
		if($q = $this->query($query)) {
			$s = $q->fetch_object();
			
			return $s->crs;
		}
		else {
			return NULL;
		}
	}
	
	public function getLocationList($loc) {
		$loc = $this->escape($loc);
		
		$query = 'SELECT DISTINCT tps_description FROM tiplocs WHERE crs != "" AND (crs = "'. $loc .'" OR tps_description LIKE "'. $loc .'%")';
		
		if($q = $this->query($query)) {
			$list = array();
			while($s = $q->fetch_object()) {
				$list[] = self::capitalizeWords($s->tps_description);
			}
			
			return $list;
		}
		else {
			return NULL;
		}
	}
	
	public function generateAssociationSchedules() {
		$query = 'SELECT * FROM associations WHERE (category IN ("JJ", "VV") AND assoc_type = "P")';
		
		if($q = $this->query($query)) {
			while($association = $q->fetch_object()) {
				// calculate the association
				$mainService = NULL;
				$assocService = NULL;
				
				$dateFromAssoc = $association->date_from;
				$dateToAssoc = $association->date_to;
				
				if($association->date_indicator == "N") {
					$dateFromAssoc = $this->getMySQLDateYMD($dateFromAssoc .'+1 day');
					$dateToAssoc = $this->getMySQLDateYMD($dateToAssoc .'+1 day');
				}
				
				
				if($association->date_indicator == "P") {
					$dateFromAssoc = $this->getMySQLDateYMD($dateFromAssoc .'-1 day');
					$dateToAssoc = $this->getMySQLDateYMD($dateToAssoc .'-1 day');
				}
				
				if($association->category == "JJ" && $association->date_indicator == "P") {
					$current_run = array($association->runs_mo,$association->runs_tu,$association->runs_we,$association->runs_th,$association->runs_fr,$association->runs_sa,$association->runs_su);
					
					$association->runs_mo = $current_run[6];
					$association->runs_tu = $current_run[0];
					$association->runs_we = $current_run[1];
					$association->runs_th = $current_run[2];
					$association->runs_fr = $current_run[3];
					$association->runs_sa = $current_run[4];
					$association->runs_su = $current_run[5];
					
					unset($current_run);
				}
				
				if($association->stp_indicator == "P") {
					$mainService = $this->getScheduleDetails($association->main_train_uid, $association->date_from, "P");
					$assocService = $this->getScheduleDetails($association->assoc_train_uid, $dateFromAssoc, "P");
				}
				else {
					$mainService = $this->getScheduleDetails($association->main_train_uid, $association->date_from);
					$assocService = $this->getScheduleDetails($association->assoc_train_uid, $dateFromAssoc);
				}
				
				if($mainService === NULL || $assocService === NULL) continue;
				
				// create the schedule
				$schedule = 		 array('display_uid' => (($association->category == "JJ") ? $association->assoc_train_uid : $association->main_train_uid),
										   'actual_uid' => $association->assoc_train_uid,
										   'date_from' => (($association->category == "JJ") ? $dateFromAssoc : $association->date_from),
										   'date_to' => (($association->category == "JJ") ? $dateToAssoc : $association->date_to),
										   'runs_mo' => $association->runs_mo,
										   'runs_tu' => $association->runs_tu,
										   'runs_we' => $association->runs_we,
										   'runs_th' => $association->runs_th,
										   'runs_fr' => $association->runs_fr,
										   'runs_sa' => $association->runs_sa,
										   'runs_su' => $association->runs_su,
										   'bank_hol' => $mainService->bank_hol,
										   'status' => $assocService->status,
										   'category' => $assocService->category,
										   'train_class' => $assocService->train_class,
										   'sleepers' => $assocService->sleepers,
										   'reservations' => $assocService->reservations,
										   'catering_code' => $assocService->catering_code,
										   'service_branding' => $assocService->service_branding,
										   'stp_indicator' => $association->stp_indicator,
										   'atoc_code' => (($association->category == "JJ") ? $assocService->atoc_code : $mainService->atoc_code));
										   
				// cycle through escaping each input
				foreach($schedule as &$val) { $val = $this->escape($val); }
				$schedule = (object)$schedule;
				$insertId = 0;
				
				
				// ok, now insert this into the table so we can get somewhat of an output
				if($this->query('INSERT INTO associations_schedules (`display_uid`, `actual_uid`, `date_from`, `date_to`, `runs_mo`, `runs_tu`, `runs_we`, `runs_th`, `runs_fr`, `runs_sa`, `runs_su`, `bank_hol`, `status`, `category`, `train_class`, `sleepers`, `reservations`, `catering_code`, `service_branding`, `stp_indicator`, `atoc_code`) VALUES ("'. $schedule->display_uid .'", "'. $schedule->actual_uid .'","'. $schedule->date_from .'","'. $schedule->date_to .'","'. $schedule->runs_mo .'","'. $schedule->runs_tu .'","'. $schedule->runs_we .'","'. $schedule->runs_th .'","'. $schedule->runs_fr .'","'. $schedule->runs_sa .'","'. $schedule->runs_su .'","'. $schedule->bank_hol .'","'. $schedule->status .'","'. $schedule->category .'","'. $schedule->train_class .'","'. $schedule->sleepers .'","'. $schedule->reservations .'","'. $schedule->catering_code .'","'. $schedule->service_branding .'","'. $schedule->stp_indicator .'","'. $schedule->atoc_code .'")')) {
					$insertId = $this->db->insert_id;
				}
				else {
					continue;
				}
				
				// insert the cancellations (stp indicator C)
				$this->query('INSERT INTO associations_schedules_stpcancel SELECT '. $insertId .' as id, cancel_from, cancel_to, cancel_mo, cancel_tu, cancel_we, cancel_th, cancel_fr, cancel_sa, cancel_su FROM associations_stpcancel WHERE associations_stpcancel.id = '. $association->id);
				
				// process the stopping pattern
				if($association->category == "JJ") {
					$this->query('insert into associations_locations select '. $insertId .' as id, @rownum:=@rownum+1 ordering, s.location_type, tiploc_code, tiploc_instance, arrival, public_arrival, pass, departure, public_departure, platform, line, path, engineering_allowance, pathing_allowance, performance_allowance, activity from ((SELECT 0 service_order, location_order, location_type, tiploc_code, tiploc_instance, arrival, "" public_arrival, pass, departure, public_departure, platform, line, path, engineering_allowance, pathing_allowance, performance_allowance, activity FROM locations assoc WHERE id = '. $this->escape($assocService->id) .' AND location_order < (SELECT location_order FROM locations WHERE id = '. $this->escape($assocService->id) .' AND tiploc_code = "'. $this->escape($association->location) .'" and tiploc_instance = "'. $this->escape($association->assoc_location_suffix) .'") AND public_departure != "") UNION (SELECT 1 service_order, location_order, location_type, tiploc_code, tiploc_instance, arrival, public_arrival, pass, departure, "" public_departure, platform, line, path, engineering_allowance, pathing_allowance, performance_allowance, activity FROM locations main WHERE id = '. $this->escape($mainService->id) .' AND location_order > (SELECT location_order FROM locations WHERE id = '. $this->escape($mainService->id) .' AND tiploc_code = "'. $this->escape($association->location) .'" AND tiploc_instance = "'. $this->escape($association->base_location_suffix) .'") AND public_arrival != "")) s, (SELECT @rownum:=-1) r  order by service_order,location_order');
				}
				else {
					$this->query('insert into associations_locations select '. $insertId .' as id, @rownum:=@rownum+1 ordering, s.location_type, tiploc_code, tiploc_instance, arrival, public_arrival, pass, departure, public_departure, platform, line, path, engineering_allowance, pathing_allowance, performance_allowance, activity from ((SELECT 0 service_order, location_order, location_type, tiploc_code, tiploc_instance, arrival, "" public_arrival, pass, departure, public_departure, platform, line, path, engineering_allowance, pathing_allowance, performance_allowance, activity FROM locations main WHERE id = '. $this->escape($mainService->id) .' AND location_order < (SELECT location_order FROM locations WHERE id = '. $this->escape($mainService->id) .' AND tiploc_code = "'. $this->escape($association->location) .'" AND tiploc_instance = "'. $this->escape($association->base_location_suffix) .'") AND public_departure != "") UNION (SELECT 1 service_order, location_order, location_type, tiploc_code, tiploc_instance, arrival, public_arrival, pass, departure, "" public_departure, platform, line, path, engineering_allowance, pathing_allowance, performance_allowance, activity FROM locations assoc WHERE id = '. $this->escape($assocService->id) .' AND location_order > (SELECT location_order FROM locations WHERE id = '. $this->escape($assocService->id) .' AND tiploc_code = "'. $this->escape($association->location) .'" and tiploc_instance = "'. $this->escape($association->assoc_location_suffix) .'") AND public_arrival != "")) s, (SELECT @rownum:=-1) r  order by service_order,location_order');
				}
			}
			
			$this->query('INSERT INTO associations_schedules_cache (`id`, `origin`, `origin_time`, `public_origin`) SELECT id, tiploc_code as origin, departure as departure, public_departure as public_origin FROM associations_locations WHERE location_type = "LO"');
			$this->query('UPDATE associations_schedules_cache, associations_locations SET associations_schedules_cache.destination = associations_locations.tiploc_code, associations_schedules_cache.destination_time = associations_locations.arrival, associations_schedules_cache.public_destination = associations_locations.public_arrival WHERE associations_locations.id = associations_schedules_cache.id AND associations_locations.location_type = "LT"');
		}
	}
	
	private function generateWhereClausesFromWhereObject($whereObj) {
		$str = array();

		if(property_exists($whereObj, 'atoc')) {
			$str[] = 'schedules.atoc_code = "'. $this->escape($whereObj->atoc) .'"';
		}
		
		if(property_exists($whereObj, 'categories') || property_exists($whereObj, 'status')) {
			$str[] = $this->generateCategoryQuery($whereObj);
		}
	}
	
	public function findDuplicateCandidates($whereObj) {
		$query = 'SELECT DISTINCT IF(schedules.train_uid < dsc.train_uid, schedules.train_uid, dsc.train_uid) lower,IF(schedules.train_uid < dsc.train_uid, dsc.train_uid, schedules.train_uid) higher, IF(schedules.train_identity = dsc.train_identity, FALSE, TRUE) AS headcodediff, IF(schedules.train_uid < dsc.train_uid, schedules.train_identity, dsc.train_identity) lowerhcd, IF(schedules.train_uid < dsc.train_uid, dsc.train_identity, schedules.train_identity) higherhcd, sc.origin, sc.origin_time, sc.destination, sc.destination_time, IF((sc.public_origin = "" AND dsc.public_origin != "") OR (sc.public_origin != "" AND dsc.public_origin = ""), TRUE, FALSE) somepublictimes, IF(schedules.stp_indicator = "P" AND dsc.stp_indicator = "N", TRUE, IF(schedules.stp_indicator = "N" AND dsc.stp_indicator = "P", TRUE, FALSE)) clashtype from schedules join schedules_cache sc on sc.id = schedules.id join (SELECT schedules.train_uid, sc.*, schedules.train_identity, schedules.service_code, schedules.stp_indicator FROM schedules LEFT JOIN schedules_stpcancel sccan ON schedules.id = sccan.id AND ("{{date}}" between cancel_from and cancel_to) AND cancel_{{day}}=1 JOIN schedules_cache sc ON sc.id = schedules.id left join schedules scstp on (schedules.train_uid = scstp.train_uid and ("{{date}}" between scstp.date_from and scstp.date_to) and scstp.runs_{{day}} = 1 and scstp.stp_indicator = "O") WHERE ("{{date}}" between schedules.date_from and schedules.date_to) and schedules.runs_{{day}}=1 AND (schedules.stp_indicator = "N" or schedules.stp_indicator = "O" or (schedules.stp_indicator = "P" AND scstp.train_uid is null AND sccan.id is null))  AND schedules.atoc_code = "{{operator}}") dsc on dsc.id != sc.id and dsc.origin = sc.origin and dsc.origin_time = sc.origin_time and dsc.destination = sc.destination and dsc.destination_time = sc.destination_time LEFT JOIN schedules_stpcancel sccan ON schedules.id = sccan.id AND ("{{date}}" between cancel_from and cancel_to) AND cancel_{{day}}=1 left join schedules scstp on (schedules.train_uid = scstp.train_uid and ("{{date}}" between scstp.date_from and scstp.date_to) and scstp.runs_{{day}} = 1 and scstp.stp_indicator = "O") WHERE ("{{date}}" between schedules.date_from and schedules.date_to) and schedules.runs_{{day}}=1 AND (schedules.stp_indicator = "N" or schedules.stp_indicator = "O" or (schedules.stp_indicator = "P" AND scstp.train_uid is null AND sccan.id is null)) AND schedules.atoc_code = "{{operator}}" ORDER BY sc.origin, sc.origin_time';

		
		// replace {{date}} with query date, {{day}} with query day (i.e. mo, tu, etc), {{operator}} atoc code
		
		$date = new DateTime();
		
		$candidates = array();
		
		$matchType = new stdClass;
		$matchType->num = 0;
		$matchType->exact = array();
		$matchType->similar = array();
				
		for($i = 0; $i < 89; $i++) {
			list($dt, $day) = $this->getMySQLDate($date->format('Y-m-d'));
						
			$q = str_replace(array('{{date}}', '{{day}}', '{{operator}}'), array($dt, $day, $this->escape($whereObj)), $query);
			
			if($out = $this->query($q)) {
				$candidates[$dt] = new stdClass;
				$candidates[$dt]->exact = array();
				$candidates[$dt]->stpdupe = clone $matchType;
				$candidates[$dt]->publictimes = clone $matchType;
				$candidates[$dt]->similar = array();
				
				while($row = $out->fetch_object()) {
					
					// grab the two services for the given date
					$main = $this->getService($row->lower, $dt, false);
					$assoc = $this->getService($row->higher, $dt, false);
				
					// hashcheck
					if($this->generateHashOfScheduleObject($main, !$row->somepublictimes) == $this->generateHashOfScheduleObject($assoc, !$row->somepublictimes)) {
						$row->exact = true;
						$row->differences = NULL;
					}
					else {
						$row->exact = false;
						$row->differences = $this->evaluateScheduleDiffs($main, $assoc, ($main->locations[0]->publicDeparture != "" && $row->somepublictimes == 0));
						
						if($row->differences === false) continue;
					}
					
					if($row->clashtype == 1) {
						$candidates[$dt]->stpdupe->num++;
						
						if($row->differences == NULL) $row->differences = array();
						
						if($main->stpIndicator == "P") {
							$row->differences[] = 'Train '. $row->higher .' is the STP schedule';
						} else {
							$row->differences[] = 'Train '. $row->lower .' is the STP schedule';
						}
					
						if($row->exact) $candidates[$dt]->stpdupe->exact[] = $row;
						else            $candidates[$dt]->stpdupe->similar[] = $row;
					}
					else if($row->somepublictimes == 1) {
						$candidates[$dt]->publictimes->num++;
						
						if($row->differences == NULL) $row->differences = array();
						
						if($main->locations[0]->publicDeparture == "") {
							$row->differences[] = 'Train '. $row->higher .' has the GBTT timings, '. $row->lower .' is likely a path that has not been cancelled';
						} else {
							$row->differences[] = 'Train '. $row->lower .' has the GBTT timings, '. $row->higher .' is likely a path that has not been cancelled';
						}
					
						if($row->exact) $candidates[$dt]->publictimes->exact[] = $row;
						else            $candidates[$dt]->publictimes->similar[] = $row;
					}
					else if($row->exact) {
						$candidates[$dt]->exact[] = $row;
					}
					else {
						$candidates[$dt]->similar[] = $row;
					}
				}
			}
			
			$date->add(new DateInterval('P1D'));
		}
				
		return $candidates;
	} 
	
	private function generateHashOfScheduleObject($schedule, $publicOnly = true) {
		// the default is to consider the public timings and platforms only as only this is relevant to the travelling public
		
		$str = "";
		
		foreach($schedule->locations as $loc) {
		
			$pfm = ($schedule->categoryCode == "BR" || $schedule->categoryCode == "BS" || $schedule->statusCode == "S" || $schedule->statusCode == "4") ? '' : $loc->platform;
			
			if($publicOnly) {
				if($loc->publicArrival != "" || $loc->publicDeparture != "") {
					$str.=sprintf('%7s%5s%5s%3s', $loc->tiploc, $loc->publicArrival, $loc->publicDeparture, $pfm);
				}
			}
			else {
				$str.=sprintf('%7s%5s%5s%3s', $loc->tiploc, $loc->arrival, $loc->departure, $pfm);
			}
		}
				
		if(!$publicOnly && strlen($str) == 0) {
			return NULL;
		}
		elseif(strlen($str) == 0) {
			return $this->generateHashOfScheduleObject($schedule, false);
		}
		else {
			return hash('sha512', $str);
		}
	}
	
	private function evaluateScheduleDiffs($main, $assoc, $public) {
		$out = $this->locations_diff($main, $assoc, $public);
		
		//printf("Output count %u, mainloc count %u, assloc %u, count %u\n", count($out), count($main->locations), count($assoc->locations), 15);
		
		if(count($out) > (count($main->locations)+(0.75*count($assoc->locations)))) { 
			return array('*** NOTE *** These schedules are likely to not be duplicates.', 'These schedules were determined to be differing too much to be considered duplicates,', 'but still may be worth considering investigation. This situation may occur', 'when there is a circular service and the times match at origin & destination.');
		}
		
		return $out;
	}
	
	private function locations_diff($main, $assoc, $public) {
		$rt = array();
		$rta = array();
		$assocLocs = $assoc->locations;
	
		foreach($main->locations as $mainloc) {
			$assocLocs = array_values($assocLocs);
			$exists = false;
			
			for($i = 0; $i < count($assocLocs); $i++) {
				$output = ($public) ? $this->comparePublicLocationSchedule($mainloc, $assocLocs[$i], $main->train_uid, $assoc->train_uid) : $this->compareInternalLocationSchedule($mainloc, $assocLocs[$i], $main->train_uid, $assoc->train_uid);
				
				if($output === true) { 
					$exists = true;
					unset($assocLocs[$i]); break;
				}
				else if(is_array($output)) { // stdClass
					$exists = true;
					$rt = array_merge($rt, $output);
				
					unset($assocLocs[$i]); break;
				}
			}
			
			if(!$exists) {
				$rta[] = sprintf("Schedule %s contains location %s which does not exist in schedule %s", $main->train_uid, $mainloc->tiploc, $assoc->train_uid);
			}
		}
		
		$rt = array_merge($rt, $rta);
		
		foreach($assocLocs as $loc) {
			$rt[] = sprintf("Schedule %s contains location %s which does not exist in schedule %s", $assoc->train_uid, $loc->tiploc, $main->train_uid);
		}
		
		return $rt;
	}
	
	private static function comparePublicLocationSchedule($a, $b, $main_uid, $assoc_uid) {
		if($a->tiploc != $b->tiploc && $a->tiplocInstance != $b->tiplocInstance) return false;
		else if($a->tiploc == $b->tiploc && $a->publicArrival == $b->publicArrival && $a->publicDeparture == $b->publicDeparture && $a->platform == $b->platform) return true;
		else if($a->tiploc == $b->tiploc && $a->tiplocInstance == $b->tiplocInstance) {
			$out = array();
		
			if($a->publicArrival != $b->publicArrival) {
				if($a->publicArrival == "" && $b->publicArrival != "") {
					$out[] = sprintf("Location %7s: %s does not call or is pick up only, %s arrives at %s", $a->tiploc, $main_uid, $assoc_uid, $b->publicArrival);
				}
				else if($a->publicArrival != "" && $b->publicArrival == "") {
					$out[] = sprintf("Location %7s: %s arrives at %s, %s does not call or is pick up only", $a->tiploc, $main_uid, $a->publicArrival, $assoc_uid);
				}
				else {
					$out[] = sprintf("Location %7s: %s arrives at %s, %s arrives at %s", $a->tiploc, $main_uid, $a->publicArrival, $assoc_uid, $b->publicArrival);
				}
			}
			
			if($a->publicDeparture != $b->publicDeparture) {
				if($a->publicDeparture == "" && $b->publicDeparture != "") {
					$out[] = sprintf("Location %7s: %s does not call or is set down only, %s departs at %s", $a->tiploc, $main_uid, $assoc_uid, $b->publicDeparture);
				}
				else if($a->publicDeparture != "" && $b->publicDeparture == "") {
					$out[] = sprintf("Location %7s: %s departs at %s, %s does not call or is set down only", $a->tiploc, $main_uid, $a->publicDeparture, $assoc_uid);
				}
				else {
					$out[] = sprintf("Location %7s: %s departs at %s, %s departs at %s", $a->tiploc, $main_uid, $a->publicDeparture, $assoc_uid, $b->publicDeparture);
				}
			}
			
			if($a->platform != $b->platform) {
				$out[] = sprintf("Location %7s: %s uses platform %s, %s uses platform %s", $a->tiploc, $main_uid, $a->platform, $assoc_uid, $b->platform);
			}
			
			return $out;
		}
		else return false;
	}
		
	private static function compareInternalLocationSchedule($a, $b, $main_uid, $assoc_uid) {	
		if($a->tiploc != $b->tiploc && $a->tiplocInstance != $b->tiplocInstance) return false;
		else if($a->tiploc == $b->tiploc && $a->arrival == $b->arrival && $a->departure == $b->departure && $a->platform == $b->platform && $a->pass == $b->pass) return true;
		else if($a->tiploc == $b->tiploc && $a->tiplocInstance == $b->tiplocInstance) {
			$out = array();
		
			if($a->arrival != $b->arrival) {
				if($a->arrival == "" && $b->arrival != "") {
					$out[] = sprintf("Location %7s: %s does not call, %s arrives at %s", $a->tiploc, $main_uid, $assoc_uid, $b->arrival);
				}
				else if($a->arrival != "" && $b->arrival == "") {
					$out[] = sprintf("Location %7s: %s arrives at %s, %s does not call", $a->tiploc, $main_uid, $a->arrival, $assoc_uid);
				}
				else {
					$out[] = sprintf("Location %7s: %s arrives at %s, %s arrives at %s", $a->tiploc, $main_uid, $a->arrival, $assoc_uid, $b->arrival);
				}
			}
			
			if($a->departure != $b->departure) {
				if($a->departure == "" && $b->departure != "") {
					$out[] = sprintf("Location %7s: %s does not call, %s departs at %s", $a->tiploc, $main_uid, $assoc_uid, $b->departure);
				}
				else if($a->departure != "" && $b->departure == "") {
					$out[] = sprintf("Location %7s: %s departs at %s, %s does not call", $a->tiploc, $main_uid, $a->departure, $assoc_uid);
				}
				else {
					$out[] = sprintf("Location %7s: %s departs at %s, %s departs at %s", $a->tiploc, $main_uid, $a->departure, $assoc_uid, $b->departure);
				}
			}
			
			if($a->platform != $b->platform) {
				$out[] = sprintf("Location %7s: %s uses platform %s, %s uses platform %s", $a->tiploc, $main_uid, $a->platform, $assoc_uid, $b->platform);
			}
			
			if($a->pass != $b->pass) {
				if($a->pass == "" && $b->pass != "") {
					$out[] = sprintf("Location %7s: %s does not pass, %s passes at %s", $a->tiploc, $main_uid, $assoc_uid, $b->pass);
				}
				else if($a->pass != "" && $b->pass == "") {
					$out[] = sprintf("Location %7s: %s passes at %s, %s does not pass", $a->tiploc, $main_uid, $a->pass, $assoc_uid);
				}
				else {
					$out[] = sprintf("Location %7s: %s passes at %s, %s passes at %s", $a->tiploc, $main_uid, $a->pass, $assoc_uid, $b->pass);
				}
			}
			
			return $out;
		}
		else return false;
	}
	
	private static function convertTimeToMinsSinceMidnight($time, $prv = 0) {
		$t = (intval(substr($time,0,2))*60) + intval(substr($time,2,2));
		if( substr($time,4,1) == "H") { $t += 0.5; }
		
		if($t < $prv) $t += 1440;
		return $t;
	}
	
	public function outputAllSchedules($atoc, $location) {
		$date = new DateTime();
		$output = array();		

		for($i = 0; $i < (90-2); $i++) {
			$date->add(new DateInterval('P1D'));
			list($day, $dt) = $this->getMySQLDate($date->format('Y-m-d'));
			list($nextDay, $nextDt) = $this->getMySQLDate($date->format('Y-m-d').' +1 day');
			
			if($result = $this->query('SELECT schedules.train_uid, schedules.train_identity, schedules_cache.origin, schedules_cache.destination, origin.tps_description origin_desc, destination.tps_description destination_desc, schedules_cache.public_origin, schedules_cache.public_destination, IF(schedules.stp_indicator = "P", "WTT", IF(schedules.stp_indicator = "O", "VAR", "STP")) stp FROM schedules LEFT JOIN schedules_stpcancel ON schedules.id = schedules_stpcancel.id AND ("'. $day .'" BETWEEN schedules_stpcancel.cancel_from AND schedules_stpcancel.cancel_to) AND schedules_stpcancel.cancel_'. $dt .' =1 LEFT JOIN schedules scstp on (schedules.train_uid = scstp.train_uid and ("'. $day .'" between scstp.date_from and scstp.date_to) and scstp.runs_'. $dt .' = 1 and scstp.stp_indicator = "O")JOIN schedules_cache ON schedules.id = schedules_cache.id LEFT JOIN tiplocs origin ON origin.tiploc = schedules_cache.origin LEFT JOIN tiplocs destination ON destination.tiploc = schedules_cache.destination WHERE ("'. $day .'" BETWEEN schedules.date_from AND schedules.date_to) AND schedules.runs_'. $dt .'=1 AND (schedules.stp_indicator = "N" OR schedules.stp_indicator = "O" OR (schedules.stp_indicator = "P" AND schedules_stpcancel.id IS NULL AND scstp.id IS NULL)) AND (schedules.category IN ("OL", "OO", "XC", "XX", "XZ", "BR", "BS") OR schedules.status IN ("S", "4")) AND public_origin >= "0200"  AND schedules.atoc_code = "'. $this->escape($atoc) .'" UNION SELECT schedules.train_uid, schedules.train_identity, schedules_cache.origin, schedules_cache.destination, origin.tps_description origin_desc, destination.tps_description destination_desc, schedules_cache.public_origin, schedules_cache.public_destination, IF(schedules.stp_indicator = "P", "WTT", IF(schedules.stp_indicator = "O", "VAR", "STP")) stp FROM schedules LEFT JOIN schedules_stpcancel ON schedules.id = schedules_stpcancel.id AND ("'. $nextDay .'" BETWEEN schedules_stpcancel.cancel_from AND schedules_stpcancel.cancel_to) AND schedules_stpcancel.cancel_'. $nextDt .' =1 LEFT JOIN schedules scstp on (schedules.train_uid = scstp.train_uid and ("'. $nextDay .'" between scstp.date_from and scstp.date_to) and scstp.runs_'. $nextDt .' = 1 and scstp.stp_indicator = "O") JOIN schedules_cache ON schedules.id = schedules_cache.id LEFT JOIN tiplocs origin ON origin.tiploc = schedules_cache.origin LEFT JOIN tiplocs destination ON destination.tiploc = schedules_cache.destination WHERE ("'. $nextDay .'" BETWEEN schedules.date_from AND schedules.date_to) AND schedules.runs_'. $nextDt .'=1 AND (schedules.stp_indicator = "N" OR schedules.stp_indicator = "O" OR (schedules.stp_indicator = "P" AND schedules_stpcancel.id IS NULL AND scstp.id IS NULL)) AND (schedules.category IN ("OL", "OO", "XC", "XX", "XZ", "BR", "BS") OR schedules.status IN ("S", "4")) AND public_origin < "0200" AND schedules.atoc_code = "'. $this->escape($atoc) .'" ORDER BY origin_desc, destination_desc, FIELD(LEFT(public_origin,2), "02", "03", "04", "05", "06", "07", "08", "09", "10", "11", "12", "13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "00", "01")')) {
				$dayout = array();
				
				while($row = $result->fetch_assoc())
					$dayout[] = $row;

				$output[$day] = $row;
			}
		}

		return $output;

	}
}
