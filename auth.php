<?php
include "lib/lib2.inc";

session_start();
db_init(null);
$result=authenticate();
session_commit();

if($result!=PASS) {
  /* Not logged in */
  print(json_encode(array('result'=>'fail',
			  'error'=>'needlogin', 
			  'info'=>'Authentication failed')));
 } else {
  $ret = array('result'=>'pass',
	       'info'=>'Authentication passed', 
	       'firstname'=>$realname[$_SESSION['username']]
	       );
  // get all lists and put them into $ret
  print(json_encode($ret));
 }
?>
