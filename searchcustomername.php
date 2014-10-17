<?php
include "lib/lib2.inc";
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
  $ret = array('result'=>'fail', 'error'=>'needlogin', 'info'=>'Need log in');
  print(json_encode($ret));
} else {
  $customers = array();
  $result = My_query("select customer_name, company
                      from customer where customer_name like ?",
		      "%".$ARGS->input."%");
  $customers=$result->fetchAll(PDO::FETCH_ASSOC);
  print(json_encode(array('result'=>'pass', 'customers'=>$customers)));
}

?>
