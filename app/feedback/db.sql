--
-- Table structure for table `answer`
--
DROP TABLE IF EXISTS `answer`;
CREATE TABLE `answer` (
  `id` int NOT NULL AUTO_INCREMENT,
  `question_id` int NOT NULL,
  `sequence` smallint NOT NULL DEFAULT '0',
  `answer` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `question_id` (`question_id`),
  CONSTRAINT `answer_ibfk_1` FOREIGN KEY (`question_id`) REFERENCES `question` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `question`
--
DROP TABLE IF EXISTS `question`;
CREATE TABLE `question` (
  `id` int NOT NULL AUTO_INCREMENT,
  `survey_id` int NOT NULL,
  `type_id` smallint NOT NULL DEFAULT '2',
  `sequence` smallint NOT NULL DEFAULT '0',
  `required` tinyint(1) NOT NULL DEFAULT '0',
  `question` varchar(256) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `survey_id` (`survey_id`),
  KEY `type_id` (`type_id`),
  CONSTRAINT `question_ibfk_1` FOREIGN KEY (`survey_id`) REFERENCES `survey` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `question_ibfk_2` FOREIGN KEY (`type_id`) REFERENCES `type` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `question_ibfk_3` FOREIGN KEY (`type_id`) REFERENCES `type` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `result`
--
DROP TABLE IF EXISTS `result`;
CREATE TABLE `result` (
  `id` int NOT NULL AUTO_INCREMENT,
  `survey_id` int NOT NULL,
  `application_contact_id` mediumint DEFAULT NULL,
  `entry_date` datetime NOT NULL,
  PRIMARY KEY (`id`),
  KEY `survey_id` (`survey_id`),
  KEY `application_contact_id` (`application_contact_id`),
  CONSTRAINT `result_ibfk_1` FOREIGN KEY (`survey_id`) REFERENCES `survey` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `result_ibfk_2` FOREIGN KEY (`application_contact_id`) REFERENCES `central`.`application_contact` (`id`) ON DELETE SET NULL ON UPDATE CASCADE,
  CONSTRAINT `result_ibfk_3` FOREIGN KEY (`application_contact_id`) REFERENCES `central`.`application_contact` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `result_answer`
--
DROP TABLE IF EXISTS `result_answer`;
CREATE TABLE `result_answer` (
  `id` int NOT NULL AUTO_INCREMENT,
  `result_id` int NOT NULL,
  `question_id` int NOT NULL,
  `answer` varchar(1000) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `result_id` (`result_id`),
  KEY `question_id` (`question_id`),
  CONSTRAINT `result_answer_ibfk_1` FOREIGN KEY (`result_id`) REFERENCES `result` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `result_answer_ibfk_2` FOREIGN KEY (`question_id`) REFERENCES `question` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `result_answer_ibfk_3` FOREIGN KEY (`question_id`) REFERENCES `question` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `survey`
--
DROP TABLE IF EXISTS `survey`;
CREATE TABLE `survey` (
  `id` int NOT NULL AUTO_INCREMENT,
  `application_contact_id` mediumint DEFAULT NULL,
  `title` varchar(128) DEFAULT NULL,
  `entry_date` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `modified_date` datetime DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `public` tinyint(1) NOT NULL DEFAULT '0',
  `anonymous` tinyint(1) NOT NULL DEFAULT '1',
  `unique` tinyint(1) NOT NULL DEFAULT '0',
  `restrict` tinyint(1) NOT NULL DEFAULT '0',
  `start_date` datetime DEFAULT NULL,
  `end_date` datetime DEFAULT NULL,
  `hash` varchar(32) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `application_contact_id` (`application_contact_id`),
  CONSTRAINT `survey_ibfk_1` FOREIGN KEY (`application_contact_id`) REFERENCES `central`.`application_contact` (`id`) ON DELETE SET NULL ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Table structure for table `type`
--
DROP TABLE IF EXISTS `type`;
CREATE TABLE `type` (
  `id` smallint NOT NULL AUTO_INCREMENT,
  `name` varchar(8) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

--
-- Dumping data for table `type`
--
LOCK TABLES `type` WRITE;
INSERT INTO `type` (`name`) VALUES ('checkbox'),('radio'),('select'),('text');
UNLOCK TABLES;
