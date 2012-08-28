<?php
include "lib/lib.inc";
// Connect database.

session_start();
db_init();
$result=authenticate();
session_commit();

if($result!=PASS) {
	/* Not logged in */
    print_login_form();
    exit();
} 

/* logged in successfully */
$message = "";
$change_result = "fail";
$username = $_SESSION['username'];
$result=My_query("select * from user where username=?",$username);
if($result->rowCount()=='0') {
	// cannot find this user, something is wrong
	print_login_form();
	exit();
}

$existing_config=$result->fetch();
if (isset($_POST['changeprofile'])) {
	$need_change = FALSE;
	$sql=array();
	$sqlvar=array();
	if ($_POST['newpassword'] and $_POST['confirmpassword']) {
		// change password
		if ($_POST['newpassword'] != $_POST['confirmpassword']) {
			$message .= 'Password mis-match';
		}else{
			$password=escape_string($_REQUEST['newpassword'], 1);
			$md5_new_pass = md5($password);
			$sql[]="password=?";
			$sqlvar[]=$md5_new_pass;
			$need_change=TRUE;
		}
	} 
	
	if ($_REQUEST['email']) {
		//change email
		$email=escape_string($_REQUEST['email']);
		
		$sql[]="email=?";
		$sqlvar[] = $email;
		$result = My_query("select username from user where email=?", $email);
		if ($result->rowCount()!=0) {
			$need_change=FALSE;
			$message .= "Email exists.";
		} else {
			$need_change=TRUE;
		}
	} 
	
	if ($_REQUEST['realname']) {
		// change realname
		$realname = trim($_REQUEST['realname']);
		$sql[]="realname=?";
		$sqlvar[]=$realname;
		$need_change=TRUE;
	}

	
	if (!$need_change) {
		// nothing has been changed.
		$message .=" Nothing changed.";
	} else {
		if (count($sql)>0) {
			$sets=implode(",", $sql);
			
			
			$stmt = $dbh->prepare("update user set $sets where username=?");
			for ($i=1; $i<=count($sqlvar); $i++) {
				$stmt->bindParam($i, $sqlvar[$i-1]);
			}
			$stmt->bindParam($i, $username);
			
			$stmt->execute();
			
			$message="Profile updated.";
			$change_result="pass";
		}
		$result=My_query("select * from user where username=?", $username);
		$existing_config=$result->fetch();
	}
}

print_html_header(HOMEPAGE);
print_page_title();

?>

<div id='main'>

        <h2>Change Profile</h2>

        <form method="post" action="<?php echo $_SERVER['PHP_SELF'];?>">
	<table border='0'>
	<tr><td>Real Name: </td><td><input type="text" name="realname"/></td>
	<td><?php echo ucwords($existing_config['realname']);?></td></tr>
    <tr><td>Email Address: </td><td><input type="text" name="email"/></td>
    <td><?php echo $existing_config['email'];?></td></tr>
	<tr><td>New Password: </td><td><input type="password" name="newpassword"/></td></tr>
	<tr><td>Confirm New Password: </td><td><input type="password" name="confirmpassword"/></td></tr>
	</table>
    <input type="submit" name="changeprofile" value="Change My Profile" />
    </form>
    <div id="message"></div>
    <?php
        if ($change_result == "fail") {
            echo "<script type='text/javascript'>showmessage('#message', '$message', 'error');</script>";
        } else {
            echo "<script type='text/javascript'>showmessage('#message', '$message', 'info');</script>";
        }
    ?>
</div> 
   </body>
</html>