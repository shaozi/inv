<?php
include "lib/lib.inc";

$message = "";
$register_result = "fail";

if (isset($_POST['register'])) {
    if (!$_POST['username'] || !$_POST['newpassword'] ||
            !$_POST['confirmpassword'] || !$_REQUEST['email'] ) {
        $message = "All fields are required.";
    } else if ($_POST['newpassword'] != $_POST['confirmpassword']) {
        $message = "Confirm password does not match new password.";
    } else {
        // Connect database.
        db_init();

        $username = mb_strtolower(escape_string($_POST['username']));
        $realname = escape_string($_POST['realname']);
        $email = escape_string($_POST['email']);
        $groupname = escape_string($_REQUEST['joingroup'],1);
        $realname = ucwords($realname);
		
        $md5_new_pass = md5($_POST['newpassword']);
        // Check matching of username
        $result=My_query("select * from user where username=? or email=?", $username, $email);
        if($result->rowCount()!='0') {
            // existing user
            $message = "Username or email exists. <a href='./reset_password.php'>Reset password</a>.";
        } else {
            //new user
            if ($realname == '') {
                echo "<font color='red'>Please fill in the missing fields</font>";
            } else {
                My_query("insert user set
		                username=?,
		                realname=?,
		                isowner=1,
		                password=?,
		                email=?",
                $username,$realname,$md5_new_pass,$email);
                if (isset($_REQUEST['joingroup']) && $groupname!='None') {
            		// sanity check
            		$result = My_query("select `group` from `user_group_mapping` where `group`=?",$groupname);
            		if ($result->rowCount()>=1) {
            			My_query("insert user_group_mapping set
                		user=?,
                		`group`=?", $username, $groupname);
            		}
            	}
                $message = "New user $username, $realname is created.";
                $register_result = "pass";
            }
        }
    }
}
?>

<?php 
print_html_header(HOMEPAGE);

?>
        <?php print_page_title();?>
        <div id='main'>
        <h2>New User Register</h2>

        <form method="post" action="<?php echo $_SERVER['PHP_SELF'];?>">
	<table border='0'>
	<tr><td>Username: </td><td><input type="text" name="username"></td></tr>
	<tr><td>Real Name: </td><td><input type="text" name="realname"/></td></tr>
    <tr><td>Email Address: </td><td><input type="text" name="email"/></td></tr>
	<tr><td>Password: </td><td><input type="password" name="newpassword"/></td></tr>
	<tr><td>Confirm Password: </td><td><input type="password" name="confirmpassword"/></td></tr>
	<tr><td>Join Group: </td><td><select name="joingroup"><option value="None">None</option>
	<?php 
	 	// Connect database.
        db_init();
		$result=My_query("select distinct `group` from `user_group_mapping`");
		while ($line=$result->fetch()) {
			if ($line['group']==NULL) continue;
			print("<option value='$line[group]'>");
			print($line['group']);
			print("</option>\n");
		}
		
	?>
	</select></td></tr>
	</table>
            <input type="submit" name="register" value="Create User" />
            
        </form>

        <p>
            <a href="index.php">Go to Inventory Database</a>
        </p>
        <?php
        if ($register_result == "fail") {
            echo "<font color=\"red\">$message</font>";
        } else {
            echo "$message";
            ?>
        <p> Login <a href="index.php">inventory database</a> now.
        </p>

            <?php
        }
        ?>
        </div>
    </body>
</html>
