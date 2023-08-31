--
-- Table structure for table `account_type`
--
DROP TABLE IF EXISTS `account_type`;
CREATE TABLE `account_type` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `type` varchar(20) NOT NULL,
  `description` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `account_type`
--
LOCK TABLES `account_type` WRITE;
INSERT INTO `account_type` (`type`,`description`) VALUES ('database','Database schema accounts.'),('mainframe','Mechanized mainframe accounts.'),('mechanized','Mechanized server accounts.'),('other','Other less known accounts.'),('midrange','Mechanized midrange accounts.'),('Logger','Provides full access to Logger.'),('Logger - read','Provides read access to Logger.'),('Logger - write','Provides write access to Logger.'),('Central SSH','Provides SSH access to servers managed under Central SSH.'),('Radial - WebSocket','Authenticates a WebSocket connection to Radial.');
UNLOCK TABLES;

--
-- Table structure for table `application`
--
DROP TABLE IF EXISTS `application`;
CREATE TABLE `application` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `creation_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `package_type_id` smallint DEFAULT NULL,
  `notify_priority_id` smallint DEFAULT NULL,
  `website` varchar(256) DEFAULT NULL,
  `login_type_id` mediumint DEFAULT NULL,
  `secure_port` tinyint(1) NOT NULL DEFAULT '1',
  `auto_register` tinyint(1) NOT NULL DEFAULT '0',
  `account_check` tinyint(1) NOT NULL DEFAULT '1',
  `dependable` tinyint(1) NOT NULL DEFAULT '0',
  `retirement_date` datetime DEFAULT NULL,
  `menu_id` smallint DEFAULT NULL,
  `description` varchar(2048) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `login_type_id` (`login_type_id`),
  KEY `notify_priority_id` (`notify_priority_id`),
  KEY `package_type_id` (`package_type_id`),
  KEY `menu_id` (`menu_id`),
  KEY `name` (`name`),
  CONSTRAINT `application_ibfk_1` FOREIGN KEY (`login_type_id`) REFERENCES `login_type` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `application_ibfk_2` FOREIGN KEY (`package_type_id`) REFERENCES `package_type` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `application_ibfk_3` FOREIGN KEY (`notify_priority_id`) REFERENCES `notify_priority` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `application_ibfk_4` FOREIGN KEY (`menu_id`) REFERENCES `menu_access` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_account`
--
DROP TABLE IF EXISTS `application_account`;
CREATE TABLE `application_account` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `application_id` mediumint NOT NULL,
  `user_id` varchar(64) DEFAULT NULL,
  `encrypt` tinyint(1) NOT NULL DEFAULT '0',
  `aes` tinyint(1) NOT NULL DEFAULT '0',
  `password` varchar(256) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  `type_id` mediumint DEFAULT NULL,
  `description` varchar(512) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `application_id` (`application_id`),
  KEY `type_id` (`type_id`),
  CONSTRAINT `application_account_ibfk_1` FOREIGN KEY (`type_id`) REFERENCES `account_type` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `application_account_ibfk_2` FOREIGN KEY (`application_id`) REFERENCES `application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_contact`
--
DROP TABLE IF EXISTS `application_contact`;
CREATE TABLE `application_contact` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `application_id` mediumint NOT NULL,
  `type_id` mediumint DEFAULT NULL,
  `contact_id` mediumint NOT NULL,
  `notify` tinyint(1) NOT NULL DEFAULT '1',
  `admin` tinyint(1) NOT NULL DEFAULT '0',
  `locked` tinyint(1) NOT NULL DEFAULT '0',
  `description` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `uni_app_id_contact_id` (`application_id`,`contact_id`),
  KEY `application_id` (`application_id`),
  KEY `type_id` (`type_id`),
  KEY `contact_id` (`contact_id`),
  CONSTRAINT `application_contact_ibfk_1` FOREIGN KEY (`application_id`) REFERENCES `application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `application_contact_ibfk_2` FOREIGN KEY (`type_id`) REFERENCES `contact_type` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `application_contact_ibfk_3` FOREIGN KEY (`contact_id`) REFERENCES `person` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_dependant`
--
DROP TABLE IF EXISTS `application_dependant`;
CREATE TABLE `application_dependant` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `application_id` mediumint NOT NULL,
  `dependant_id` mediumint NOT NULL,
  PRIMARY KEY (`id`),
  KEY `application_id` (`application_id`),
  KEY `dependant_id` (`dependant_id`),
  CONSTRAINT `application_dependant_ibfk_1` FOREIGN KEY (`application_id`) REFERENCES `application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `application_dependant_ibfk_2` FOREIGN KEY (`dependant_id`) REFERENCES `application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_issue`
--
DROP TABLE IF EXISTS `application_issue`;
CREATE TABLE `application_issue` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `application_id` mediumint NOT NULL,
  `summary` varchar(256) DEFAULT NULL,
  `open_date` datetime NOT NULL,
  `close_date` datetime DEFAULT NULL,
  `due_date` datetime DEFAULT NULL,
  `priority` tinyint NOT NULL,
  `hold` tinyint(1) NOT NULL DEFAULT '0',
  `release_date` datetime DEFAULT NULL,
  `assigned_id` mediumint DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `application_id` (`application_id`),
  KEY `open_date` (`open_date`),
  KEY `close_date` (`close_date`),
  KEY `due_date` (`due_date`),
  KEY `assigned_id` (`assigned_id`),
  CONSTRAINT `application_issue_ibfk_1` FOREIGN KEY (`application_id`) REFERENCES `application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `application_issue_ibfk_2` FOREIGN KEY (`assigned_id`) REFERENCES `person` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_server`
--
DROP TABLE IF EXISTS `application_server`;
CREATE TABLE `application_server` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `application_id` mediumint NOT NULL,
  `server_id` mediumint NOT NULL,
  `description` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `application_id` (`application_id`),
  KEY `server_id` (`server_id`),
  CONSTRAINT `application_server_ibfk_1` FOREIGN KEY (`application_id`) REFERENCES `application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `application_server_ibfk_2` FOREIGN KEY (`server_id`) REFERENCES `server` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_server_contact`
--
DROP TABLE IF EXISTS `application_server_contact`;
CREATE TABLE `application_server_contact` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `application_server_id` mediumint NOT NULL,
  `application_contact_id` mediumint NOT NULL,
  PRIMARY KEY (`id`),
  KEY `application_server_id` (`application_server_id`),
  KEY `application_contact_id` (`application_contact_id`),
  CONSTRAINT `application_server_contact_ibfk_1` FOREIGN KEY (`application_contact_id`) REFERENCES `application_contact` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `application_server_contact_ibfk_2` FOREIGN KEY (`application_server_id`) REFERENCES `application_server` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `application_server_detail`
--
DROP TABLE IF EXISTS `application_server_detail`;
CREATE TABLE `application_server_detail` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `application_server_id` mediumint NOT NULL,
  `version` varchar(20) DEFAULT NULL,
  `daemon` varchar(32) DEFAULT NULL,
  `owner` varchar(32) DEFAULT NULL,
  `script` varchar(256) DEFAULT NULL,
  `delay` mediumint NOT NULL DEFAULT '0',
  `min_processes` mediumint NOT NULL DEFAULT '0',
  `max_processes` mediumint NOT NULL DEFAULT '0',
  `min_image` int NOT NULL DEFAULT '0',
  `max_image` int NOT NULL DEFAULT '0',
  `min_resident` int NOT NULL DEFAULT '0',
  `max_resident` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `application_server_id` (`application_server_id`),
  CONSTRAINT `application_server_detail_ibfk_1` FOREIGN KEY (`application_server_id`) REFERENCES `application_server` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `browser`
--
DROP TABLE IF EXISTS `browser`;
CREATE TABLE `browser` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `search` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `browser`
--
LOCK TABLES `browser` WRITE;
INSERT INTO `browser` (`name`, `search`) VALUES ('Unknown','Unknown'),('Firefox','Firefox'),('Iceweasel','Iceweasel'),('Microsoft IE','Trident'),('Safari','Safari'),('Opera','Opera'),('SeaMonkey','SeaMonkey'),('Thunderbird','Thunderbird'),('Netscape','Netscape'),('Chrome','Chrome'),('Vivaldi','Vivaldi'),('Chromium','Chromium');
UNLOCK TABLES;

--
-- Table structure for table `contact_type`
--
DROP TABLE IF EXISTS `contact_type`;
CREATE TABLE `contact_type` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `type` varchar(20) NOT NULL,
  `description` varchar(256) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `contact_type`
--
LOCK TABLES `contact_type` WRITE;
INSERT INTO `contact_type` (`type`,`description`) VALUES ('Primary Admin','Primary Administrator'),('Backup Admin','Backup Administrator'),('Primary Developer','Primary Developer'),('Backup Developer','Backup Developer'),('Contact','Customer Contact'),('Primary Contact','Primary Customer Contact');
UNLOCK TABLES;

--
-- Table structure for table `issue_comment`
--
DROP TABLE IF EXISTS `issue_comment`;
CREATE TABLE `issue_comment` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `issue_id` mediumint NOT NULL,
  `entry_date` datetime NOT NULL,
  `user_id` mediumint DEFAULT NULL,
  `comments` text,
  PRIMARY KEY (`id`),
  KEY `issue_id` (`issue_id`),
  KEY `user_id` (`user_id`),
  CONSTRAINT `issue_comment_ibfk_1` FOREIGN KEY (`issue_id`) REFERENCES `application_issue` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `issue_comment_ibfk_2` FOREIGN KEY (`user_id`) REFERENCES `person` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `login_type`
--
DROP TABLE IF EXISTS `login_type`;
CREATE TABLE `login_type` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `type` varchar(20) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `login_type`
--
LOCK TABLES `login_type` WRITE;
INSERT INTO `login_type` (`type`) VALUES ('Windows'),('OpenID'),('Password');
UNLOCK TABLES;

--
-- Table structure for table `menu_access`
--
DROP TABLE IF EXISTS `menu_access`;
CREATE TABLE `menu_access` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `type` varchar(10) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `menu_access`
--
LOCK TABLES `menu_access` WRITE;
INSERT INTO `menu_access` (`type`) VALUES ('None'),('Registered'),('Public');
UNLOCK TABLES;

--
-- Table structure for table `notify_priority`
--
DROP TABLE IF EXISTS `notify_priority`;
CREATE TABLE `notify_priority` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `priority` varchar(64) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `notify_priority`
--
LOCK TABLES `notify_priority` WRITE;
INSERT INTO `notify_priority` (`priority`) VALUES ('notify during downtime'),('notify for all changes');
UNLOCK TABLES;

--
-- Table structure for table `operating_system`
--
DROP TABLE IF EXISTS `operating_system`;
CREATE TABLE `operating_system` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `search` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `operating_system`
--
LOCK TABLES `operating_system` WRITE;
INSERT INTO `operating_system` (`name`, `search`) VALUES ('Unknown','Unknown'),('Ubuntu','Ubuntu'),('Macintosh','Macintosh'),('Windows','Windows'),('iPhone','iPhone'),('Android','Android');
UNLOCK TABLES;

--
-- Table structure for table `package_type`
--
DROP TABLE IF EXISTS `package_type`;
CREATE TABLE `package_type` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `type` varchar(64) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `package_type`
--
LOCK TABLES `package_type` WRITE;
INSERT INTO `package_type` (`type`) VALUES ('Compiler'),('Database'),('Interpretive Language'),('Library'),('Operating System'),('Terminal Application'),('Version Control'),('Web Server'),('Other');
UNLOCK TABLES;

--
-- Table structure for table `person`
--
DROP TABLE IF EXISTS `person`;
CREATE TABLE `person` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `userid` varchar(10) NOT NULL,
  `first_name` varchar(40) DEFAULT NULL,
  `last_name` varchar(60) DEFAULT NULL,
  `email` varchar(80) DEFAULT NULL,
  `pager` varchar(80) DEFAULT NULL,
  `active` tinyint(1) NOT NULL,
  `admin` tinyint(1) NOT NULL,
  `locked` tinyint(1) NOT NULL,
  `change` tinyint(1) DEFAULT NULL,
  `password` varchar(256) CHARACTER SET latin1 COLLATE latin1_swedish_ci DEFAULT NULL,
  `openid` varchar(512) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `userid` (`userid`) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `php_session`
--
DROP TABLE IF EXISTS `php_session`;
CREATE TABLE `php_session` (
  `session_id` varchar(32) NOT NULL,
  `last_updated` datetime NOT NULL,
  `session_data` longtext,
  `session_json` longtext,
  PRIMARY KEY (`session_id`),
  KEY `last_updated` (`last_updated`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Table structure for table `robot`
--
DROP TABLE IF EXISTS `robot`;
CREATE TABLE `robot` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `name` varchar(32) NOT NULL,
  `search` varchar(32) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `robot`
--
LOCK TABLES `robot` WRITE;
INSERT INTO `robot` (`name`,`search`) VALUES ('Unknown','Unknown'),('Perl','perl'),('Node','node'),('PHP','PHP'),('Java','Java'),('Subversion','SVN'),('cURL','libcurl'),('Python','python');
UNLOCK TABLES;

--
-- Table structure for table `server`
--
DROP TABLE IF EXISTS `server`;
CREATE TABLE `server` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `parent_id` mediumint DEFAULT NULL,
  `name` varchar(64) NOT NULL,
  `address` varchar(128) DEFAULT NULL,
  `location` varchar(128) DEFAULT NULL,
  `city` varchar(64) DEFAULT NULL,
  `state` varchar(2) DEFAULT NULL,
  `zipcode` varchar(10) DEFAULT NULL,
  `description` varchar(256) DEFAULT NULL,
  `processes` mediumint NOT NULL DEFAULT '0',
  `cpu_usage` smallint NOT NULL DEFAULT '0',
  `main_memory` smallint NOT NULL DEFAULT '0',
  `swap_memory` smallint NOT NULL DEFAULT '0',
  `disk_size` smallint NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `name` (`name`),
  KEY `parent_id` (`parent_id`),
  CONSTRAINT `server_ibfk_1` FOREIGN KEY (`parent_id`) REFERENCES `server` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `server_contact`
--
DROP TABLE IF EXISTS `server_contact`;
CREATE TABLE `server_contact` (
  `id` mediumint NOT NULL AUTO_INCREMENT,
  `server_id` mediumint NOT NULL,
  `type_id` mediumint NOT NULL DEFAULT '0',
  `contact_id` mediumint NOT NULL,
  `physical_access` tinyint(1) NOT NULL DEFAULT '0',
  `notify` tinyint(1) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`),
  KEY `type_id` (`type_id`),
  KEY `contact_id` (`contact_id`),
  KEY `server_id` (`server_id`),
  CONSTRAINT `server_contact_ibfk_1` FOREIGN KEY (`server_id`) REFERENCES `server` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `server_contact_ibfk_2` FOREIGN KEY (`contact_id`) REFERENCES `person` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `server_contact_ibfk_3` FOREIGN KEY (`type_id`) REFERENCES `contact_type` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `statistic`
--
DROP TABLE IF EXISTS `statistic`;
CREATE TABLE `statistic` (
  `id` int NOT NULL AUTO_INCREMENT,
  `application_id` mediumint NOT NULL,
  `browser_id` smallint NOT NULL,
  `operating_system_id` smallint NOT NULL,
  `robot_id` smallint NOT NULL,
  `ip_address` varchar(39) DEFAULT NULL,
  `date` date NOT NULL,
  `count` int NOT NULL,
  PRIMARY KEY (`id`),
  KEY `application_id` (`application_id`),
  KEY `browser_id` (`browser_id`),
  KEY `operating_system_id` (`operating_system_id`),
  KEY `robot_id` (`robot_id`),
  CONSTRAINT `statistic_ibfk_1` FOREIGN KEY (`browser_id`) REFERENCES `browser` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `statistic_ibfk_2` FOREIGN KEY (`operating_system_id`) REFERENCES `operating_system` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `statistic_ibfk_3` FOREIGN KEY (`robot_id`) REFERENCES `robot` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `statistic_ibfk_4` FOREIGN KEY (`application_id`) REFERENCES `application` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
