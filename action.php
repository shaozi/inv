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
   switch($ARGS->action) {
      case 'get_part_w_serial':
      get_part_w_serial($ARGS->serial);
      break;
      case 'get_all_parts':
      get_all_parts();
      break;
      default:
      print(json_encode(array('result'=>'fail',
			      'error'=>'unknown action',
			      'info'=>'cannot recognize action '. $ARGS->action)
			));
  }
 }
?>
