<?php
$create_table_sql = <<<END
DROP TABLE IF EXISTS `customer`;
CREATE TABLE IF NOT EXISTS `customer` (
  `customer_id` int(11) NOT NULL AUTO_INCREMENT,
  `customer_name` varchar(64) NOT NULL,
  `company` varchar(64) NOT NULL,
  `email` varchar(64) DEFAULT NULL,
  `telephone` char(10) DEFAULT NULL,
  PRIMARY KEY (`customer_id`)
) ENGINE=InnoDB  DEFAULT CHARSET=latin1;

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


DROP TABLE IF EXISTS `user`;
CREATE TABLE IF NOT EXISTS `user` (
  `username` varchar(32) NOT NULL,
  `password` char(32) NOT NULL,
  `realname` varchar(64) NOT NULL,
  `email` varchar(64) NOT NULL,
  `phone` varchar(32) NOT NULL,
  `isowner` tinyint(4) NOT NULL DEFAULT '1',
  `issuperuser` tinyint(4) NOT NULL DEFAULT '0',
  `write_group` varchar(320) NOT NULL,
  `read_group` varchar(320) NOT NULL,
  PRIMARY KEY (`username`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;


DROP TABLE IF EXISTS `user_group_mapping`;
CREATE TABLE IF NOT EXISTS `user_group_mapping` (
  `group` varchar(32) DEFAULT NULL,
  `user` varchar(32) DEFAULT NULL,
  `is_admin` tinyint(4) NOT NULL DEFAULT '0',
  UNIQUE KEY `uc_ug` (`user`,`group`),
  KEY `group` (`group`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

ALTER TABLE `user_group_mapping`
  ADD CONSTRAINT `user_group_mapping_ibfk_2` FOREIGN KEY (`user`) REFERENCES `user` (`username`) ON DELETE CASCADE ON UPDATE CASCADE;
END;



function install() {
	global $create_table_sql;
	/*
	 * 1. if inv db exists, upgrade
	 * 2. 
	 */
	$host=$_REQUEST['dbhost'];
	$user=$_REQUEST['dbuser'];
	$pass=$_REQUEST['dbpass'];
	$db=$_REQUEST['dbname'];

	$username = trim($_REQUEST['username']);
    $realname = trim($_REQUEST['realname']);
    $email = trim($_REQUEST['email']);
	$realname = ucwords($realname);
		
    $pass1 = md5($_REQUEST['pass1']);
	$pass2 = md5($_REQUEST['pass2']);
	if (strlen($username) == 0 || strlen($realname)== 0 || strlen($email) == 0 ||
		($pass1 != $pass2)) {
		return false;
	}
	try {
		$dbh = new PDO("mysql:host=$host;dbname=$db", $user, $pass);
		$stmt = $dbh->prepare($create_table_sql);
		if (!$stmt->execute()) {
			die("cannot create table");
		}
		
		
		#create superuser
		$stmt = $dbh->prepare("insert user (`username`, `password`, `realname`, `email`, `issuperuser`)
			values (:username, :password, :realname, :email, 1)");
		$stmt->bindParam(':username', $username, PDO::PARAM_STR);
		$stmt->bindParam(':password', $pass1, PDO::PARAM_STR);
		$stmt->bindParam(':realname', $realname, PDO::PARAM_STR);
		$stmt->bindParam(':email', $email, PDO::PARAM_STR);
	
		if (!$stmt->execute()) {
			die("Cannot create supper user". print_r($dbh->errorInfo(), true));
		}
		
		$config_file = fopen(dirname($_SERVER['SCRIPT_FILENAME']).'/config.php', 'w');
		fwrite($config_file, <<<CONFIG
<?php
define("DBHOST", '$host');
define("DBNAME", '$db');
define("DBUSER", '$user');
define("DBPASS", '$pass');
?>
CONFIG
		);
		fclose($config_file);
    
	} catch (PDOException $e) {
		die("DB ERROR: ". $e->getMessage());
	}
	return true;
}

if (file_exists("./config.php")) {
	die("config.php exist. database tables are already installed. Delete config.php if you want to 
		delete all tables and re-install");
}
if (isset($_REQUEST['install'])) {
	if (install()) {
		#echo "Installation Complete.";
		header('Location: '.dirname($_SERVER['SCRIPT_NAME']));
	}
}
?>

<html>
<head>
</head>
<body>
	<h1>Installation</h1>
	<div>
		<form>
			<h2>Step 1. Database Access Information</h2>
			<p>
			<table>
				<tr><td>DB name:</td><td><input type = 'text' name='dbname' /></td></tr>
				<tr><td>DB host:</td><td><input type = 'text' name='dbhost' /></td></tr>
				<tr><td>DB user:</td><td><input type = 'text' name='dbuser' /></td></tr>
				<tr><td>Password:</td><td><input type = 'password' name='dbpass' /></td></tr>
			</table>
			</p>
			
			<h2>Step 2. Create a Superuser</h2>
			<p>A superuser is the first user of the inventory database.
			Beside regular user privelege, a superuser is responsible for:
			<ul>
			<li> creating/deleting groups</li>
			<li> Assigning/un-assigning user to groups</li>
			<li> Active/deactive user</li>
			</ul></p>
			<table border='0'>
				<tr>
					<td>Superuser username:</td>
					<td><input type='text' name='username' /></td>
				</tr>
				<tr>
					<td>Real name:</td>
					<td><input type='text' name='realname' /></td>
				</tr>
				<tr>
					<td>Email address:</td>
					<td><input type='text' name='email' /></td>
				</tr>
				<tr>
					<td>Password:</td>
					<td><input type='password' name='pass1' /></td>
				</tr>
				<tr>
					<td>Confirm password:</td>
					<td><input type='password' name='pass2' /></td>
				</tr>
			</table>
			
			<h2>Step 3. Click the Install Button</h2>
				<input type="submit" name='install' value="Install"/>
			
		</form>
	</div>

</body>
</html>
