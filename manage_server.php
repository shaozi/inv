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
 * users: username->{realname: str, active:0|1, groups: group list}
 * groups: groupname->{user:user, isadmin:0|1} list
 */
if($result!=PASS) {
	/* Not logged in */
	$ret=array('result'=>FAIL, 'message'=>'Login failed!');
	print(json_encode($ret));
	exit;
} else {
	$stmt=My_query("select * from user where username=? and issuperuser=1",$_SESSION['username']);
	if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 'message'=>'Only superuser can manage groups and users.');
					print(json_encode($ret));
					exit;
				}
	if (isset($_REQUEST['action'])) {
		$action = trim($_REQUEST['action']);
		switch($action){
			case 'creategroup':
				if(isset($_REQUEST['groupname'])) {
					$groupname = trim($_REQUEST['groupname']);
					if ($groupname=='') {
						$ret=array('result'=>FAIL, 
						'message'=>'groupname cannot be empty.');
						print(json_encode($ret));
						exit;
					}
					$stmt = My_query("select * from user_group_mapping where `group`=?", $groupname);
					if ($stmt->rowCount()>=1) {
						$ret=array('result'=>FAIL, 
						'message'=>"Group already exist.");
						print(json_encode($ret));
						exit;
					}
					My_query("insert into user_group_mapping set `group`=?", $groupname);
					$ret=array('result'=>PASS, 
						'message'=>"Group is successfully created.");
						print(json_encode($ret));
						exit;
				} else {
					$ret=array('result'=>FAIL, 
					'message'=>'groupname is missing.');
					print(json_encode($ret));
					exit;
				}
				break;
				
			case 'deleteuser':
				if(isset($_REQUEST['username']) && isset($_REQUEST['transferusername'])) {
					$username = trim($_REQUEST['username']);
					$transferusername = trim($_REQUEST['transferusername']);
					if ($transferusername=='' || $username=='' || $transferusername==$username) {
						$ret=array('result'=>FAIL, 
						'message'=>'two usernames cannot be empty or same.');
						print(json_encode($ret));
						exit;
					}
					$stmt=My_query("select * from user where username=?", $username);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"User to be deleted does not exist.");
						print(json_encode($ret));
						exit;
					}
					$stmt=My_query("select * from user where username=?", $transferusername);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"User to be transferred to does not exist.");
						print(json_encode($ret));
						exit;
					}
					My_query("delete from user_group_mapping where `user`=?",
					 $username);
					
					My_query("update parts set owner=? where 
					`owner`=?", 
					$transferusername, $username);
					
					$stmt=My_query("update user set isowner=0 where 
					`username`=?", 
					$username);
					
					$ret=array('result'=>PASS, 
						'message'=>"User is deactivated.");
						print(json_encode($ret));
						exit;
				} else {
					$ret=array('result'=>FAIL, 
					'message'=>'groupname and/or username are missing.');
					print(json_encode($ret));
					exit;
				}
				break;
			case 'addusertogroup':
				if(isset($_REQUEST['groupname']) && isset($_REQUEST['username'])) {
					$groupname = trim($_REQUEST['groupname']);
					$username = trim($_REQUEST['username']);
					if ($groupname=='' || $username=='') {
						$ret=array('result'=>FAIL, 
						'message'=>'groupname or username cannot be empty.');
						print(json_encode($ret));
						exit;
					}
					$stmt=My_query("select * from user where username=?", $username);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"User $username does not exist.");
						print(json_encode($ret));
						exit;
					
					}
					$stmt=My_query("select * from user_group_mapping where `group`=?",
					 $groupname);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"Group $groupname does not exist.");
						print(json_encode($ret));
						exit;
					
					}
					$stmt=My_query("select * from user_group_mapping where 
					`group`=? and user=?", 
					$groupname, $username);
					if ($stmt->rowCount()>=1) {
						$ret=array('result'=>FAIL, 
						'message'=>"User $username is already in Group $groupname.");
						print(json_encode($ret));
						exit;
					}
					My_query("insert into user_group_mapping 
					set `group`=?, user=?", 
					$groupname, $username);
					$ret=array('result'=>PASS, 
						'message'=>"User $username is added to Group $groupname.");
						print(json_encode($ret));
						exit;
				} else {
					$ret=array('result'=>FAIL, 
					'message'=>'groupname and/or username are missing.');
					print(json_encode($ret));
					exit;
				}
				break;
				
			case 'deleteuserfromgroup':
				if(isset($_REQUEST['groupname']) && isset($_REQUEST['username'])) {
					$groupname = trim($_REQUEST['groupname']);
					$username = trim($_REQUEST['username']);
					if ($groupname=='' || $username=='') {
						$ret=array('result'=>FAIL, 
						'message'=>'groupname or username cannot be empty.');
						print(json_encode($ret));
						exit;
					}
					$stmt=My_query("select * from user where username=?", $username);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"User $username does not exist.");
						print(json_encode($ret));
						exit;
					
					}
					$stmt=My_query("select * from user_group_mapping 
					where `group`=? and user=?",
					 $groupname, $username);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"User $username is not in Group $group.");
						print(json_encode($ret));
						exit;
					
					}
					
					My_query("delete from user_group_mapping 
					where `group`=? and user=?", 
					$groupname, $username);
					$ret=array('result'=>PASS, 
						'message'=>"User $username is deleted from Group $groupname.");
						print(json_encode($ret));
						exit;
				} else {
					$ret=array('result'=>FAIL, 
					'message'=>'groupname and/or username are missing.');
					print(json_encode($ret));
					exit;
				}
				break;
			case 'deletegroup':
				if(isset($_REQUEST['groupname'])) {
					$groupname = trim($_REQUEST['groupname']);
					if ($groupname=='') {
						$ret=array('result'=>FAIL, 
						'message'=>'groupname cannot be empty.');
						print(json_encode($ret));
						exit;
					}
					$stmt=My_query("select * from user_group_mapping where `group`=?", $groupname);
					if ($stmt->rowCount()<1) {
						$ret=array('result'=>FAIL, 
						'message'=>"Group $groupname doesnot exist.");
						print(json_encode($ret));
						exit;
					}
					My_query("delete from user_group_mapping where `group`=?", $groupname);
					$ret=array('result'=>PASS, 
						'message'=>"Group $groupname is successfully deleted.");
						print(json_encode($ret));
						exit;
				} else {
					$ret=array('result'=>FAIL, 
					'message'=>'groupname is missing.');
					print(json_encode($ret));
					exit;
				}
				break;
			case 'toggleusergroupadmin':
				if(isset($_REQUEST['groupname']) && isset($_REQUEST['username'])) {
					$groupname = trim($_REQUEST['groupname']);
					$username = trim($_REQUEST['username']);
					if ($groupname=='' || $username=='') {
						$ret=array('result'=>FAIL, 
						'message'=>'groupname or username cannot be empty.');
						print(json_encode($ret));
						exit;
					}
					$stmt=My_query("select * from user where username=?", $username);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"User $username does not exist.");
						print(json_encode($ret));
						exit;
					
					}
					$result = My_query("select * from user_group_mapping 
					where `group`=? and user=?",
					 $groupname, $username);
					if ($stmt->rowCount()<1) {
					$ret=array('result'=>FAIL, 
						'message'=>"User $username is not in Group $group.");
						print(json_encode($ret));
						exit;
					
					}
					$line=$result->fetch();
					if ($line['is_admin']==1) {
						My_query("update user_group_mapping set is_admin=0
						where `group`=? and user=?", 
						$groupname, $username);
						$message="becomes a regular user";
					} else {
						My_query("update user_group_mapping set is_admin=1
						where `group`=? and user=?", 
						$groupname, $username);
						$message = "becomes an admin";
					}
					$ret=array('result'=>PASS, 
						'message'=>"User $username $message of $groupname.");
						print(json_encode($ret));
						exit;
				} else {
					$ret=array('result'=>FAIL, 
					'message'=>'groupname and/or username are missing.');
					print(json_encode($ret));
					exit;
				}
				break;
			case 'listusergroup':
				
				$ret=array('result'=>PASS, 'users'=>array(), 'groups'=>array());

				$result = My_query("select username, realname, isowner from user");
				while($line=$result->fetch()) {
					$username=$line['username'];
					$realname=ucwords($line['realname']);
					$ret['users'][$username]=array();
					$ret['users'][$username]['realname']=$realname;
					$ret['users'][$username]['active']=$line['isowner'];
					
					$ret['users'][$username]['groups']=array();
					$result1 = My_query("select `group` from user_group_mapping
						where `group` is not NULL
						and user=?", $username);
					while($line1 = $result1->fetch()) {
						$ret['users'][$username]['groups'][] = $line1['group'];
					}
				}

				$result= My_query("select distinct `group` from user_group_mapping
					where `group` is not NULL");
				while($line=$result->fetch()) {
					$groupname=$line['group'];
					$ret['groups'][$groupname] = array();
					$result1=My_query("select user,is_admin from user_group_mapping where
							`group`=?
							and user is not NULL", $groupname);
					while($line1=$result1->fetch()) {
						$ret['groups'][$groupname][] = 
							array('user'=>$line1['user'], 'isadmin'=>$line1['is_admin']);
					}
				}
				print(json_encode($ret));
				break;
			default:
				print(json_encode(array('result'=>FAIL,'message'=>'Unknown action')));
				break;
		}
	}

}

?>
