-- phpMyAdmin SQL Dump
-- version 3.3.9
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Generation Time: May 17, 2011 at 08:05 PM
-- Server version: 5.5.8
-- PHP Version: 5.3.5

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Database: `inv`
--
DROP DATABASE IF EXISTS `inv`;
CREATE DATABASE `inv` DEFAULT CHARACTER SET latin1 COLLATE latin1_swedish_ci;
USE `inv`;

-- --------------------------------------------------------

--
-- Table structure for table `customer`
--

DROP TABLE IF EXISTS `customer`;
CREATE TABLE IF NOT EXISTS `customer` (
  `customer_id` int(11) NOT NULL AUTO_INCREMENT,
  `customer_name` varchar(64) NOT NULL,
  `company` varchar(64) NOT NULL,
  `email` varchar(64) DEFAULT NULL,
  `telephone` char(10) DEFAULT NULL,
  PRIMARY KEY (`customer_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `parts`
--

DROP TABLE IF EXISTS `parts`;
CREATE TABLE IF NOT EXISTS `parts` (
  `serial` varchar(32) NOT NULL,
  `model` varchar(32) NOT NULL,
  `part_number` varchar(32) NOT NULL,
  `status` set('in','out') NOT NULL DEFAULT 'in',
  `part_comment` text,
  `transaction_id` int(11) NOT NULL,
  `owner` varchar(32) NOT NULL,
  PRIMARY KEY (`serial`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `transaction`
--

DROP TABLE IF EXISTS `transaction`;
CREATE TABLE IF NOT EXISTS `transaction` (
  `serial` varchar(32) NOT NULL,
  `customer_id` int(11) DEFAULT NULL,
  `action` set('in','out','request_checkin','request_checkout','approve','deny') NOT NULL,
  `last_transaction_id` int(11) NOT NULL,
  `date` timestamp NOT NULL DEFAULT '0000-00-00 00:00:00',
  `duebackin` int(11) NOT NULL DEFAULT '30' COMMENT 'Due back time when check out an item (in days)',
  `username` varchar(64) NOT NULL,
  `transaction_comment` text NOT NULL,
  `transaction_id` int(11) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`transaction_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `user`
--

DROP TABLE IF EXISTS `user`;
CREATE TABLE IF NOT EXISTS `user` (
  `username` varchar(32) NOT NULL,
  `password` char(32) NOT NULL,
  `realname` varchar(64) NOT NULL,
  `email` varchar(64) NOT NULL,
  `phone` varchar(32) NOT NULL,
  `isowner` tinyint(4) NOT NULL DEFAULT '1',
  `issupperuser` tinyint(4) NOT NULL DEFAULT '0',
  `write_group` varchar(320) NOT NULL,
  `read_group` varchar(320) NOT NULL,
  PRIMARY KEY (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Table structure for table `user_group_mapping`
--

DROP TABLE IF EXISTS `user_group_mapping`;
CREATE TABLE IF NOT EXISTS `user_group_mapping` (
  `group` varchar(32) DEFAULT NULL,
  `user` varchar(32) DEFAULT NULL,
  `is_admin` tinyint(4) NOT NULL DEFAULT '0',
  UNIQUE KEY `uc_ug` (`user`,`group`),
  KEY `group` (`group`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Constraints for dumped tables
--

--
-- Constraints for table `user_group_mapping`
--
ALTER TABLE `user_group_mapping`
  ADD CONSTRAINT `user_group_mapping_ibfk_2` FOREIGN KEY (`user`) REFERENCES `user` (`username`) ON DELETE CASCADE ON UPDATE CASCADE;
