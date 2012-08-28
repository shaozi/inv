<?php

function install() {
	/*
	 * 1. if inv db exists, upgrade
	 * 2. 
	 */
	$host="localhost";

	$root="root";
	$root_password=$_REQUEST['rootpass'];

	$user='inv';
	$pass='1nv1NV';
	$db="inv";

	try {
		$dbh = new PDO("mysql:host=$host", $root, $root_password);
			
		$dbh->exec("CREATE DATABASE `$db`;
					CREATE USER '$user'@'localhost' IDENTIFIED BY '$pass';
					GRANT ALL ON `$db`.* TO '$user'@'localhost';
					FLUSH PRIVILEGES;") 
		or die(print_r($db->errorInfo(), true));

	} catch (PDOException $e) {
		die("DB ERROR: ". $e->getMessage());
	}
}
?>

<html>
<head>
</head>
<body>
	<h1>Installation/Upgrade</h1>
	<div>
		<form>

			<h2>Step 1. Create a Superuser for Inventory Database</h2>
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
					<td><input type='text' name='invsuperuserusername' /></td>
				</tr>
				<tr>
					<td>Real name:</td>
					<td><input type='text' name='invsuperuserrealname' /></td>
				</tr>
				<tr>
					<td>Email address:</td>
					<td><input type='text' name='invsuperuseremail' /></td>
				</tr>
				<tr>
					<td>Password:</td>
					<td><input type='text' name='invsuperuserpass1' /></td>
				</tr>
				<tr>
					<td>Confirm password:</td>
					<td><input type='text' name='invsuperuserpass2' /></td>
				</tr>
			</table>

			<h2>Step 2. Now provide the Mysql root password and click Install
				button</h2>
			<p>
				Mysql root password: <input type='text' name='rootpass' /> <br /> <input
					type='submit' name='install' value='Install' />
			</p>
		</form>
	</div>

</body>
</html>
