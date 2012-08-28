<?php
include "lib/lib.inc";

$message = "";
$reset_password_result = "fail";

if (isset($_POST['resetpassword'])) {
    if (!$_POST['email'] ) {
        $message = "Type your email address and click Reset Password button.";
    } else {
        // Connect database.
        db_init();
        $email = $_POST['email'];
     	$random_pass = chr(rand(65,90)).chr(rand(65,90)).chr(rand(65,90)).chr(rand(65,90)).chr(rand(65,90));
        
        $md5_random_pass = md5($random_pass);
        // Check matching of username
        $result=My_query("select * from user where email=? and isowner=1", $email);
        if($result->rowCount()=='0') {
            $message = "Email does not exists or user is deactivated. <a href='./register.php'>Register as New User</a>.";
        } else {
        	//change password
        	/*
        	 * email from PC is blocked by mcafee. change on access scan property to allow httpd.exe to send email.
        	 */
        	My_query("update user set
		                password=?
		                where email=?", $md5_random_pass, $email);
        	$to      = $email;
        	$subject = 'Inventory DB Password Reset';
        	$message = "Your new password is $random_pass";
        	$headers = "From: jchen@ixiacom.com" . "\r\n" .
    						'Reply-To: jchen@ixiacom.com' . "\r\n" .
    						'X-Mailer: PHP/' . phpversion();
        	mail($to, $subject, $message, $headers);

        	$message = "Password has been reset. New password has been sent to $email.";
        	$reset_password_result = "pass";
        }
    }
}
print_html_header(HOMEPAGE); 
print_page_title();
?>
        <div id="main">
        <h2>Reset Password</h2>

        <form method="post" action="<?php echo $_SERVER['PHP_SELF'];?>">
	<table border='0'>
    <tr><td>Email Address: </td><td><input type="text" name="email"/></td></tr>
	</table>
            <input type="submit" name="resetpassword" value="Reset Password" />
            
        </form>

        <p>
            <a href="./index.php">Go to Inventory Database</a>
        </p>
        <?php
        if ($reset_password_result == "fail") {
            echo "<font color=\"red\">$message</font>";
        } else {
            echo "$message";
        }
        ?>
        </div>
    </body>
</html>