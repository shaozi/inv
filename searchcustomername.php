<?php
include "lib/lib.inc";
session_start();
db_init();
$result=authenticate();
session_commit();
/*
 * return structure:
 * result: 1 PASS, 0 FAIL
 * message: (string) reason of failure
 * users: username->group list
 * groups: groupname->user list
 */
$ret=array();
if($result!=PASS) {
    /* Not logged in */
	
	print(json_encode($ret));
	exit;
} else {
	$term = escape_string($_REQUEST['term']);
	
	$result = My_query("select * from customer where
	customer_name like ?",  "$term%");
	while($line=$result->fetch()) {
		$name=$line['customer_name'];
		$company=$line['company'];
		$ret[]=array('name'=>ucwords($name), 'company'=>ucwords($company));
	}
	print(json_encode($ret));
}

?>
