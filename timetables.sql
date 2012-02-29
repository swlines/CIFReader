CREATE TABLE `associations` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `main_train_uid` char(6) NOT NULL,
  `assoc_train_uid` char(6) NOT NULL,
  `date_from` date NOT NULL,
  `date_to` date NOT NULL,
  `runs_mo` tinyint(1) NOT NULL,
  `runs_tu` tinyint(1) NOT NULL,
  `runs_we` tinyint(1) NOT NULL,
  `runs_th` tinyint(1) NOT NULL,
  `runs_fr` tinyint(1) NOT NULL,
  `runs_sa` tinyint(1) NOT NULL,
  `runs_su` tinyint(1) NOT NULL,
  `category` enum('JJ','VV','NP') NOT NULL,
  `date_indicator` enum('S','N','P') NOT NULL,
  `location` char(7) NOT NULL,
  `base_location_suffix` enum('2','3','4','5','6','7','8','9') NOT NULL,
  `assoc_location_suffix` enum('2','3','4','5','6','7','8','9') NOT NULL,
  `assoc_type` enum('O','P') NOT NULL,
  `stp_indicator` enum('P','O','N') NOT NULL,
  PRIMARY KEY (`id`),
  KEY `date_from` (`date_from`),
  KEY `date_to` (`date_to`),
  KEY `assoc_mo` (`runs_mo`),
  KEY `assoc_tu` (`runs_tu`),
  KEY `assoc_we` (`runs_we`),
  KEY `assoc_th` (`runs_th`),
  KEY `assoc_fr` (`runs_fr`),
  KEY `assoc_sa` (`runs_sa`),
  KEY `assoc_su` (`runs_su`),
  KEY `location` (`location`),
  KEY `main_train_uid` (`main_train_uid`),
  KEY `assoc_train_uid` (`assoc_train_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `associations_stpcancel` (
  `id` int(11) NOT NULL,
  `cancel_from` date NOT NULL,
  `cancel_to` date NOT NULL,
  `cancel_mo` tinyint(1) NOT NULL,
  `cancel_tu` tinyint(1) NOT NULL,
  `cancel_we` tinyint(1) NOT NULL,
  `cancel_th` tinyint(1) NOT NULL,
  `cancel_fr` tinyint(1) NOT NULL,
  `cancel_sa` tinyint(1) NOT NULL,
  `cancel_su` tinyint(1) NOT NULL,
  KEY `id` (`id`),
  KEY `cancel_mo` (`cancel_mo`),
  KEY `cancel_tu` (`cancel_tu`),
  KEY `cancel_we` (`cancel_we`),
  KEY `cancel_th` (`cancel_th`),
  KEY `cancel_fr` (`cancel_fr`),
  KEY `cancel_sa` (`cancel_sa`),
  KEY `cancel_su` (`cancel_su`),
  KEY `cancel_from` (`cancel_from`),
  KEY `cancel_to` (`cancel_to`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Contains LTP services with an STP cancel (C on CIF)';

CREATE TABLE `associations_stpcancel_core` (
  `main_train_uid` char(6) NOT NULL DEFAULT '',
  `assoc_train_uid` char(6) NOT NULL,
  `location` char(7) NOT NULL,
  `base_location_suffix` enum('2','3','4','5','6','7','8','9') NOT NULL,
  `assoc_location_suffix` enum('2','3','4','5','6','7','8','9') NOT NULL,
  `cancel_from` date NOT NULL,
  `cancel_to` date NOT NULL,
  `cancel_mo` tinyint(1) NOT NULL,
  `cancel_tu` tinyint(1) NOT NULL,
  `cancel_we` tinyint(1) NOT NULL,
  `cancel_th` tinyint(1) NOT NULL,
  `cancel_fr` tinyint(1) NOT NULL,
  `cancel_sa` tinyint(1) NOT NULL,
  `cancel_su` tinyint(1) NOT NULL,
  PRIMARY KEY (`main_train_uid`,`assoc_train_uid`,`location`,`base_location_suffix`,`assoc_location_suffix`,`cancel_from`),
  KEY `cancel_mo` (`cancel_mo`),
  KEY `cancel_tu` (`cancel_tu`),
  KEY `cancel_we` (`cancel_we`),
  KEY `cancel_th` (`cancel_th`),
  KEY `cancel_fr` (`cancel_fr`),
  KEY `cancel_sa` (`cancel_sa`),
  KEY `cancel_su` (`cancel_su`),
  KEY `cancel_from` (`cancel_from`),
  KEY `cancel_to` (`cancel_to`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Contains LTP services with an STP cancel (C on CIF)';

CREATE TABLE `locations` (
  `id` int(11) NOT NULL,
  `location_order` smallint(6) NOT NULL,
  `location_type` enum('LO','LI','LT') NOT NULL,
  `tiploc_code` char(7) NOT NULL,
  `tiploc_instance` enum('2','3','4','5','6','7','8','9') NOT NULL,
  `arrival` char(5) NOT NULL,
  `public_arrival` char(4) NOT NULL,
  `pass` char(5) NOT NULL,
  `departure` char(5) NOT NULL,
  `public_departure` char(4) NOT NULL,
  `platform` char(3) NOT NULL,
  `line` char(3) NOT NULL,
  `path` char(3) NOT NULL,
  `engineering_allowance` char(2) NOT NULL,
  `pathing_allowance` char(2) NOT NULL,
  `performance_allowance` char(2) NOT NULL,
  `activity` char(12) NOT NULL,
  `public_call` tinyint(1) NOT NULL DEFAULT '0',
  `actual_call` tinyint(1) NOT NULL DEFAULT '0',
  `order_time` char(5) NOT NULL,
  `act_a` tinyint(1) NOT NULL,
  `act_ae` tinyint(1) NOT NULL,
  `act_bl` tinyint(1) NOT NULL,
  `act_c` tinyint(1) NOT NULL,
  `act_d` tinyint(1) NOT NULL,
  `act_minusd` tinyint(1) NOT NULL,
  `act_e` tinyint(1) NOT NULL,
  `act_g` tinyint(1) NOT NULL,
  `act_h` tinyint(1) NOT NULL,
  `act_hh` tinyint(1) NOT NULL,
  `act_k` tinyint(1) NOT NULL,
  `act_kc` tinyint(1) NOT NULL,
  `act_ke` tinyint(1) NOT NULL,
  `act_kf` tinyint(1) NOT NULL,
  `act_ks` tinyint(1) NOT NULL,
  `act_l` tinyint(1) NOT NULL,
  `act_n` tinyint(1) NOT NULL,
  `act_op` tinyint(1) NOT NULL,
  `act_or` tinyint(1) NOT NULL,
  `act_pr` tinyint(1) NOT NULL,
  `act_r` tinyint(1) NOT NULL,
  `act_rm` tinyint(1) NOT NULL,
  `act_rr` tinyint(1) NOT NULL,
  `act_s` tinyint(1) NOT NULL,
  `act_t` tinyint(1) NOT NULL,
  `act_minust` tinyint(1) NOT NULL,
  `act_tb` tinyint(1) NOT NULL,
  `act_tf` tinyint(1) NOT NULL,
  `act_ts` tinyint(1) NOT NULL,
  `act_tw` tinyint(1) NOT NULL,
  `act_u` tinyint(1) NOT NULL,
  `act_minusu` tinyint(1) NOT NULL,
  `act_w` tinyint(1) NOT NULL,
  `act_x` tinyint(1) NOT NULL,
  KEY `id` (`id`),
  KEY `location_type` (`location_type`),
  KEY `tiploc_code` (`tiploc_code`),
  KEY `order_time` (`order_time`),
  KEY `public_call` (`public_call`),
  KEY `actual_call` (`actual_call`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `locations_alternatives` (
  `tiploc` char(7) NOT NULL DEFAULT '',
  `additional_tiploc` char(7) NOT NULL DEFAULT '',
  UNIQUE KEY `additional_tiploc` (`additional_tiploc`),
  KEY `tiploc` (`tiploc`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `locations_change` (
  `id` int(11) NOT NULL,
  `tiploc` char(7) NOT NULL,
  `tiploc_instance` char(1) NOT NULL,
  `category` char(2) NOT NULL,
  `train_identity` char(4) NOT NULL,
  `headcode` char(4) NOT NULL,
  `service_code` char(8) NOT NULL,
  `portion_id` enum('1','2','4','8','Z') NOT NULL,
  `power_type` enum('D','DEM','DMU','E','ED','EML','EMU','EPU','HST','LDS') NOT NULL,
  `timing_load` char(4) NOT NULL,
  `speed` char(3) NOT NULL,
  `operating_characteristics` char(6) NOT NULL,
  `train_class` enum('B','S') NOT NULL,
  `sleepers` enum('B','S','F') NOT NULL,
  `reservations` enum('A','R','S','E') NOT NULL,
  `catering_code` char(4) NOT NULL,
  `service_branding` char(4) NOT NULL,
  `uic_code` char(5) NOT NULL,
  `rsid` char(8) NOT NULL,
  KEY `id` (`id`),
  KEY `identity` (`train_identity`),
  KEY `tiploc` (`tiploc`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;


CREATE TABLE `schedules` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `unique_id` char(15) NOT NULL,
  `train_uid` char(6) NOT NULL,
  `date_from` date NOT NULL,
  `date_to` date NOT NULL,
  `runs_mo` tinyint(1) NOT NULL,
  `runs_tu` tinyint(1) NOT NULL,
  `runs_we` tinyint(1) NOT NULL,
  `runs_th` tinyint(1) NOT NULL,
  `runs_fr` tinyint(1) NOT NULL,
  `runs_sa` tinyint(1) NOT NULL,
  `runs_su` tinyint(1) NOT NULL,
  `bank_hol` enum('X','G') NOT NULL,
  `status` enum('1','2','3','4','5','B','F','P','S','T') NOT NULL,
  `category` char(2) NOT NULL,
  `train_identity` char(4) NOT NULL,
  `headcode` char(4) NOT NULL,
  `service_code` char(8) NOT NULL,
  `portion_id` enum('1','2','4','8','Z') NOT NULL,
  `power_type` enum('D','DEM','DMU','E','ED','EML','EMU','EPU','HST','LDS') NOT NULL,
  `timing_load` char(4) NOT NULL,
  `speed` char(3) NOT NULL,
  `operating_characteristics` char(6) NOT NULL,
  `train_class` enum('B','S') NOT NULL,
  `sleepers` enum('B','S','F') NOT NULL,
  `reservations` enum('A','R','S','E') NOT NULL,
  `catering_code` char(4) NOT NULL,
  `service_branding` char(4) NOT NULL,
  `stp_indicator` enum('P','N','O') NOT NULL,
  `uic_code` char(5) NOT NULL,
  `atoc_code` char(2) NOT NULL,
  `ats_code` enum('Y','N') NOT NULL,
  `rsid` char(8) NOT NULL,
  `data_source` enum('0','T') NOT NULL,
  `bus` tinyint(1) NOT NULL DEFAULT '0',
  `train` tinyint(1) NOT NULL DEFAULT '0',
  `ship` tinyint(1) NOT NULL DEFAULT '0',
  `passenger` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `train_uid` (`train_uid`),
  KEY `date_from` (`date_from`),
  KEY `date_to` (`date_to`),
  KEY `runs_mo` (`runs_mo`),
  KEY `runs_tu` (`runs_tu`),
  KEY `runs_we` (`runs_we`),
  KEY `runs_th` (`runs_th`),
  KEY `runs_fr` (`runs_fr`),
  KEY `runs_sa` (`runs_sa`),
  KEY `runs_su` (`runs_su`),
  KEY `stp_indicator` (`stp_indicator`),
  KEY `train_identity` (`train_identity`),
  KEY `bank_hol` (`bank_hol`),
  KEY `status` (`status`),
  KEY `atoc_code` (`atoc_code`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `schedules_cache` (
  `id` int(11) NOT NULL,
  `origin` char(7) NOT NULL,
  `origin_time` char(5) NOT NULL,
  `public_origin` char(4) NOT NULL,
  `destination` char(7) NOT NULL,
  `destination_time` char(5) NOT NULL,
  `public_destination` char(4) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  KEY `origin` (`origin`,`destination`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `schedules_stpcancel` (
  `id` int(11) NOT NULL,
  `cancel_from` date NOT NULL,
  `cancel_to` date NOT NULL,
  `cancel_mo` tinyint(1) NOT NULL,
  `cancel_tu` tinyint(1) NOT NULL,
  `cancel_we` tinyint(1) NOT NULL,
  `cancel_th` tinyint(1) NOT NULL,
  `cancel_fr` tinyint(1) NOT NULL,
  `cancel_sa` tinyint(1) NOT NULL,
  `cancel_su` tinyint(1) NOT NULL,
  KEY `id` (`id`),
  KEY `cancel_from` (`cancel_from`),
  KEY `cancel_to` (`cancel_to`),
  KEY `cancel_mo` (`cancel_mo`),
  KEY `cancel_tu` (`cancel_tu`),
  KEY `cancel_we` (`cancel_we`),
  KEY `cancel_th` (`cancel_th`),
  KEY `cancel_fr` (`cancel_fr`),
  KEY `cancel_sa` (`cancel_sa`),
  KEY `cancel_su` (`cancel_su`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Contains LTP services with an STP cancel (C on CIF)';

CREATE TABLE `schedules_stpcancel_core` (
  `train_uid` char(6) NOT NULL,
  `cancel_from` date NOT NULL,
  `cancel_to` date NOT NULL,
  `cancel_mo` tinyint(1) NOT NULL,
  `cancel_tu` tinyint(1) NOT NULL,
  `cancel_we` tinyint(1) NOT NULL,
  `cancel_th` tinyint(1) NOT NULL,
  `cancel_fr` tinyint(1) NOT NULL,
  `cancel_sa` tinyint(1) NOT NULL,
  `cancel_su` tinyint(1) NOT NULL,
  PRIMARY KEY (`train_uid`,`cancel_from`),
  KEY `cancel_from` (`cancel_from`),
  KEY `cancel_to` (`cancel_to`),
  KEY `cancel_mo` (`cancel_mo`),
  KEY `cancel_tu` (`cancel_tu`),
  KEY `cancel_we` (`cancel_we`),
  KEY `cancel_th` (`cancel_th`),
  KEY `cancel_fr` (`cancel_fr`),
  KEY `cancel_sa` (`cancel_sa`),
  KEY `cancel_su` (`cancel_su`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Contains LTP services with an STP cancel (C on CIF)';

CREATE TABLE `tiplocs` (
  `tiploc` char(7) NOT NULL,
  `nalco` char(6) NOT NULL,
  `tps_description` char(60) NOT NULL DEFAULT '',
  `stanox` char(5) NOT NULL,
  `crs` char(3) NOT NULL,
  `description` char(16) NOT NULL,
  PRIMARY KEY (`tiploc`),
  KEY `crs_code` (`crs`),
  KEY `stanox` (`stanox`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `tiplocs_cache` (
  `tiploc` char(7) NOT NULL,
  `nalco` char(6) NOT NULL,
  `tps_description` char(50) NOT NULL,
  `stanox` char(5) NOT NULL,
  `crs` char(3) NOT NULL,
  `description` char(16) NOT NULL,
  PRIMARY KEY (`tiploc`),
  KEY `crs_code` (`crs`),
  KEY `stanox` (`stanox`),
  FULLTEXT KEY `tps_description` (`tps_description`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `updates` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `file` char(7) NOT NULL DEFAULT '',
  `user` char(6) NOT NULL,
  `date_from` date NOT NULL,
  `date_to` date NOT NULL,
  `update_type` enum('F','U') NOT NULL,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `stations` (
  `tiploc` char(7) NOT NULL DEFAULT '',
  `crs` char(3) NOT NULL DEFAULT '',
  `description` varchar(50) NOT NULL,
  PRIMARY KEY (`tiploc`),
  UNIQUE KEY `crs` (`crs`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;