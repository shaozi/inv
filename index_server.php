<?php
include "lib/lib.inc";
session_start();
$dbh=db_init();
$result=authenticate($dbh);
session_commit();
/*
 * return structure:
 * result: 1 PASS, 0 FAIL
 * message: (string) reason of failure
 * count: how many request are waiting.
 */
if($result!=PASS) {
	/* Not logged in */
	$ret=array('result'=>FAIL, 'message'=>'Login failed!');
	print(json_encode($ret));
	exit;
} else {
	if (isset($_REQUEST['action'])) {
		$action = trim($_REQUEST['action']);
		switch($action){
			case 'get_request_count':
					
				$ret=array('result'=>PASS,
						'count'=>count_approval());
				print(json_encode($ret));
				exit;

				break;
			case 'approve' :
				if (isset($_REQUEST['serial'])) {
					$ret = approve_request($_REQUEST['serial']);
					print(json_encode(array('result'=>$ret)));
				} else {
					print(json_encode(array('result'=>FAIL,
											'message'=>'missing serial')));
				}
				break;
			case 'deny' :
				if (isset($_REQUEST['serial'])) {
					$ret = deny_request($_REQUEST['serial']);
					print(json_encode(array('result'=>$ret)));
				} else {
					print(json_encode(array('result'=>FAIL,
											'message'=>'missing serial')));
				}
				break;
			default:
				print(json_encode(array('result'=>FAIL,
										'message'=>'Unknown action')));
				break;
		}
	}

}

?>
