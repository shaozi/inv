<?php
include "lib/lib.inc";
session_start();
db_init();
$result=authenticate();
session_commit();
/*
 * term: serial
 * return structure:
 * result: 1 PASS, 0 FAIL
 * message: (string) reason of failure
 * parts: {serial, model} list
 */
$ret=array();
if($result!=PASS) {
    /* Not logged in */
	
	print(json_encode($ret));
	exit;
} else {
	$term = escape_string($_REQUEST['term']);
	$result = My_query("select * from parts where
	serial like ?", "$term%");
	while($line=$result->fetch()) {
		$serial=$line['serial'];
		$model=$line['model'];
		$ret[]=array('serial'=>strtoupper($serial), 'model'=>strtoupper($model));
	}
	print(json_encode($ret));
}

?>
