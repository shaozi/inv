<?php
/*
 * SQL and other libraries used in Inventory database
 *
 */
$db_var = array();
$realname = array();
$get_info_done = FALSE;
$dbh;
$config_file = dirname($_SERVER['SCRIPT_FILENAME']).'/config.php';
if (!file_exists($config_file)) {
	die("Cannot find config.php. Run install.php first");
}

include $config_file;

define("NONE_EXIST", -1);
define("RIGHT_NONE", 0);
define("RIGHT_RO", 1);
define("RIGHT_FULL", 2);
define('PASS', 1);
define('FAIL', 0);
define('DEBUG', 0);

define('HOMEPAGE', 'Inventory Database');
define('MANAGEPAGE', 'Inventory Database - User Group Management');
define('CHANGEPROFILEPAGE', 'Inventory Database - Change Profile');
define('HELP', 'Inventory Database - User Guide');

$SELECT_DB_MAPPING = array(
        "owner"=>array("sql"=>"parts.owner", "desc"=>"Owner"),
        "comment"=>array("sql"=>"comment","desc"=>"Comment"),
        "duebackdate"=>array("sql"=>"duebackdate", "desc"=>"Due Back Date"),
        "model"=>array("sql"=>"parts.model", "desc"=>"Model"),
        "transactiondate"=>array("sql"=>"transaction.date desc", "desc"=>"Transaction Date"),
        "customer"=>array("sql"=>"customer.customer_name", "desc"=>"Customer Name")
);

$ARGS = json_decode(file_get_contents("php://input"));

function db_lock_all_tables() {
  My_query("LOCK TABLES
		    user WRITE, 
		    parts WRITE,
	            transaction WRITE, 
	            customer WRITE,
	            user_group_mapping WRITE,
	            user_group_mapping as ugmap WRITE");
}


function db_init() {
  global $dbh;
  global $realname;
  // Connect database.
  $host=DBHOST; // Host name.
  $db_user=DBNAME; // MySQL username.
  $db_password=DBPASS; // MySQL password.
  $database=DBNAME; // Database name.
  try {
    $dbh = new PDO("mysql:host=$host;dbname=$database", DBUSER, DBPASS );
    foreach ($dbh->query('select username,realname from user') as $row) {
      $realname[$row['username']] = ucwords($row['realname']);
    }
    return $dbh;
  } catch (PDOException $e) {
    print("<pre>DB ERROR: ". $e->getMessage());
    print( "\n To reinstall database, remove config.php and go to install.php </pre>");
    die();
  }
}

function authenticate() {
  /*
   * Use session to save username. Very simple authentication. However, session
   * cookie is sent over http every time.
   * Volnerable to sniffing
   *
   * A better solution is to use localstorage to store a rsa key then each time
   * the server will send a challenge to the client. Client will use its stored key
   * to encrypt the challenge and send it back to the server to verify.
   *
   */
  global $dbh;
  global $ARGS;

  if(isset($_SESSION['username'])) {
    //echo "auth with session";
    
    $stmt = $dbh->prepare("select *
	        from user where username=:username and isowner=1");
    $stmt->bindParam(':username', $_SESSION['username']);
    $stmt->execute();
    if ($stmt->rowCount()==1) {
      if ($ARGS->action =='logout') {
	clear_session();
	return FAIL;
      }
      return PASS;
    } else {
      clear_session();
      return FAIL;
    }
  }
  if ( $ARGS->action=='login') {
      $username = strtolower(trim($ARGS->username));
      $password = trim($ARGS->password);
      
      $stmt = $dbh->prepare("select * from user where
			username=:username and password=:password and isowner=1");
      
      $stmt->bindParam(':username', $username);
      $md5_pass = md5($password);
      $stmt->bindParam(':password', $md5_pass );
      $stmt->execute();
      if ($stmt->rowCount()==1) {
	// auth passed
	$_SESSION['username'] = $username;
	return PASS;
      }
  }
  return FAIL;
}



//
// MySQL Query
// usage:
//    $query such as "SELECT * FROM users WHERE nick = ? AND pass = ?"
//      where ? are the placeholders for the parameters
// call:
//    $__->My_query( $query, arg0, arg1, arg2, ... )
// description:
//    The parameters given to the function are escaped through mysql_real_escape_string(),
//    merged with the query and then given to the mysql_query function.
// returns:
//    Array of lines returned by the MySQL Database
//


function My_query() {
	global $dbh;

	$numParams = func_num_args();
	$params = func_get_args();

	$stmt = $dbh->prepare($params[0]);

	if ($numParams > 1) {
		for ($i = 1; $i < $numParams; $i++){
			$stmt->bindParam($i, $params[$i]);
		}
	}

	if (DEBUG)  {
		print("<div><pre>");
		print ("numParams = $numParams\n");
		print_r($params);
			
		$stmt->debugDumpParams();
		print("</div></pre>");
	}
	$stmt->execute();
	return $stmt;
}

function My_query_ret() {
	global $dbh;

	$numParams = func_num_args();
	$params = func_get_args();

	$stmt = $dbh->prepare($params[0]);

	if ($numParams > 1) {
		for ($i = 1; $i < $numParams; $i++){
			$stmt->bindParam($i, $params[$i]);
		}
	}

	if (DEBUG)  {
		print("<div><pre>");
		print ("numParams = $numParams\n");
		print_r($params);
			
		$stmt->debugDumpParams();
		print("</div></pre>");
	}
	$stmt->execute();
	$ret = $stmt->fetch();
	return $ret[0];
}


function escape_string($s, $keep_case = 1) {
	if (! isset($s)) return '';
	return trim($s);
}

function print_error_msg($msg) {
	print("<div style='height:32px'><div style='float:left;' class='error'>$msg</div></div>");
}
function print_info_msg($msg) {
	print("<div style='height:32px'><div style='float:left;' class='info'>$msg</div></div>");
}

function check_permission($username, $serial) {
	/*
	 * check_permission checks if a user can view, check in/out
	 * a part.
	 *
	 * permission is determined by the owner and group
	 * The owner and admin of the owner's group have same previllege
	 * to check in check out and approve request from other users
	 * in the same group to a part. User within the same group of
	 * the owner can view the part. When they check in or out the part,
	 * a request is sent to the owner and the admin of the group.
	 * The owner and the admin of the group can approve or reject that
	 * request.
	 *
	 * returns an array:
	 * 	ret[approvals] if user can get the card but need approval, this is the list. Otherwise, NULL
	 *  ret[permission] is a number representing the permission:
	 * 		NONE_EXIST (-1): part does not exist
	 * 		RIGHT_NONE (0): user cannot view the part
	 * 		RIGHT_RO (1): user can view the part but cannot write the part
	 * 		RIGHT_FULL (2): user own full rights of the part.
	 *
	 */

	$result = My_query("select owner from parts where serial=?", $serial);
	if ($result->rowCount()==0) {
		$ret['approvals']=NULL;
		$ret['permission']=NONE_EXIST;
	}
	$line =  $result->fetch();
	$owner = $line['owner'];
	if ($owner==$username) {
		$ret['approvals']=$owner;
		$ret['permission']=RIGHT_FULL;
		return $ret;
	}

	$q = "select distinct `group` from user_group_mapping
    	where user=? 
    	and `group` in 
    		(select `group` from user_group_mapping as ugmap where user=?)";
	$result = My_query($q, $username, $owner);
	if ($result->rowCount()==0) {
		$ret['approvals']=NULL;
		$ret['permission']=RIGHT_NONE;
		return $ret;
	}
	while ($line = $result->fetch()) {
		$common_groups[]= $line['group'];
	}

	# owner has right to approve request
	$approvals[]=$owner;
	# any admin in the common groups can also approve
	foreach ($common_groups as $group) {
		$q = "select distinct `user` from `user_group_mapping` where `group`=? and is_admin=1";
		$result = My_query($q, $group);
		while ($line =  $result->fetch()) {
			if ($line['user']!=NULL && ! in_array($line['user'], $approvals)) {
				$approvals[] = $line['user'];
			}
		}
	}

	if (in_array($username, $approvals)) {
		# owner or admin of the common group has full control
		$ret['approvals']=$approvals;
		$ret['permission']=RIGHT_FULL;
		return $ret;
	}

	$ret['approvals'] = $approvals;
	$ret['permission'] = RIGHT_RO;
	return $ret;
}

function get_same_group_users ($username) {
	$users[]=$username;
	$q = "select distinct `user` from user_group_mapping where `group` in
    	(select `group` from user_group_mapping as ugmap where 
    	user=? and `group` is not NULL) and user is not NULL";
	$result = My_query($q, $_SESSION['username']);
	while ($line = $result->fetch()) {
		if (!in_array($line['user'], $users)) $users[] = $line['user'];
	}
	return $users;
}

function deletepart($info=array()) {
	global $realname;
	if ($info['serial'] == '') {
		print_error_msg("ERROR: serial is empty!");
		return;
	}
	// sanity check
	$result = My_query("select serial from parts where
                      serial=?", $info['serial']);
	if ($result->rowCount() == 0) {
		print_error_msg("ERROR: no parts found");
		return;
	}
	// check owner
	$permit = check_permission($_SESSION['username'], $info['serial']);

	if ($permit['permission']==NONE_EXIST) {
		$info['err_message'] = "$model with serial number ".
		strtoupper($serial). " does not exist. You cannot delete it.";
		print_transact_form($info);
		return;
	}
	if ($permit['permission']!=RIGHT_FULL) {
		$info['err_message'] = "$model with serial number ".
		strtoupper($serial). " does not belong to you. You cannot delete it.";
		print_transact_form($info);
		return;
	}
	db_lock_all_tables();
	My_query("DELETE from parts where serial=?", $info['serial']);
	My_query("DELETE from transaction where serial=?", $info['serial']);
	print_info_msg("$info[serial] is deleted");
	My_query("UNLOCK TABLES");
}

function updatepart($info=array()) {
	/*print("<pre>");
	 print_r($info);
	 print("</pre>");*/
	global $realname;
	if ($info['serial'] == '') {
		print_error_msg("ERROR: serial is empty!");
		return;
	}

	// sanity check
	if ($info['old_serial'] == '') {
		$info['old_serial'] = $info['serial'];
	}
	$result = My_query("select serial from parts where
                      serial='$info[old_serial]'");
	if ($result->rowCount()==0) {
		print_error_msg("ERROR: no parts found");
		return;
	}
	// check owner
	$permit = check_permission($_SESSION['username'], $info['old_serial']);

	if ($permit['permission']!=RIGHT_FULL) {
		$info['err_message'] = "Permission violation. Edit failed.";
		print_transact_form($info);
		return FALSE;
	}


	db_lock_all_tables();
	if ($info['serial'] != $info['old_serial']) {
		// update serial
		// first check if serial exists
		$sql_res = My_query("select serial from parts
	                 where serial=?", $info['serial']);
		if ($sql_res->rowCount() != 0) {
			print_error_msg("ERROR: Parts with SN=$info[serial] exists.");
			My_query("UNLOCK TABLES");
			return;
		}
		// update
		My_query("update parts,transaction set parts.serial=?,
                               parts.model=?,
                               parts.owner=?,
                               parts.part_comment=?,
                               transaction.transaction_comment=?
              where parts.serial = ? AND parts.transaction_id = transaction.transaction_id",
		$info['serial'], $info['model'], $info['owner'], 
		$info['comment'],
		$info['trans_comment'], 
		$info['old_serial']);
		My_query("update transaction set serial=?
              where serial=?", $info['serial'], $info['old_serial']);
	} else {
		// update
		My_query("update parts,transaction set parts.model=?,
                               parts.owner=?,
                               parts.part_comment=?,
                               transaction.transaction_comment=?
              where parts.serial =? AND parts.transaction_id = transaction.transaction_id",
		$info['model'], $info['owner'], $info['comment'], 
		$info['trans_comment'], $info['old_serial']);
	}
	My_query("UNLOCK TABLES");
	print_info_msg("Part updated.");
	showdetail($info);
}

function print_new_checkin_form($info = array()) {
	global $realname;
	//print_r($info);
	if (isset($info['err_message'])) {
		print_error_msg("ERROR: $info[err_message]");
	} else {
		print_info_msg("This item is not in database. <br />
       Please fill the following form and
       click <b>Add to Database</b> button to add it into the database.");
	}
	if (isset($info['message'])) {
		print_info_msg("$info[message]");
	}
	print <<<END
	
    <form action='./index.php' name='new_checkin' method='post'>
    <p>
    <label for='serial'>Serial Number:</label>
    <br />
    <input type='text' class='itext' name='serial' id='serial'
           value='$_POST[serial]'/>
    <br />
    <label for='model'>Model Name:</label>
    <br />
    <input type='text' class='itext' name='model' id='model'
           value='$_POST[model]'/>
    </p>
    <p>
    <label for='owner'>Part Owner: </label>
    <br />
    <select class='itext' name='owner' id='owner'>
END;

	$users=get_same_group_users($info['username']);

	//print_r($users);

	foreach ($users as $owner_name) {
		echo "<option value='$owner_name'>" . $realname[$owner_name] . "</option>";
	}
	print <<<END
	</select>
     </p>

     <p>
     <label for='comment'>Comment:</label>
     <br />
     <textarea rows='2' cols='30' class='itext' name='comment' id='comment'>$_POST[comment]</textarea>
     </p>
     <p>
     <input type='submit' class='btn btn-default' name='new_checkin' value=' Add to Database ' />
     <input type='submit' class='btn btn-default' name='cancel' value=' Cancel ' />
     </p>
     </form>
END;

}


/**
 database struture

 parts.serial,
 parts.model,
 parts.owner,
 parts.borrower as borrower,
 customer.customer_name as customer,
 transaction.transaction_comment as comment,
 transaction.username as username,
 transaction.date as date,
 transaction.duebackin as duebackin
 from parts JOIN ( transaction, customer ) ON
 ( parts.serial = transaction.serial AND
 transaction.customer_id = customer.customer_id AND
 parts.transaction_id = transaction.transaction_id )
 where
 parts.serial regexp '$info[serial]' and parts.status='out'
 and ( (parts.owner in $sql_grps)
 or (parts.borrower in $sql_grps) )
 ");
 */



function print_search_result($info=array()) {
	global $realname;
	global $SELECT_DB_MAPPING;
	$MYTSTP = time();
	$in = 0;
	$out = 0;
	$result_in = null;
	$result_out = null;

	$username=$_SESSION['username'];

	$user_restrict_subq = "select distinct `user` from user_group_mapping
    		 where  (`group` in (select `group` from user_group_mapping as ugmap
    		                     where user=? and `group` is not NULL)) 
    	         or (user=? and `group` is NULL)";


	if ($info['serial']) {
		$search_critiria = " parts.serial regexp ? ";
		$search_var = $info['serial'];
	} else if ($info['model']) {
		$search_critiria = " parts.model regexp ? ";
		$search_var = $info['model'];
	} else if ($info['customer']) {
		$search_critiria = " customer.customer_name regexp ? ";
		$search_var=$info['customer'];
	} else if ($info['company']) {
		$search_critiria = " customer.company regexp ? ";
		$search_var = $info['company'];
	} else {
		$info['err_message'] = "What do you want to search?";
		print_transact_form($info);
		return;
	}


	if ($info['serial'] || $info['model'] ) {
		/*
		 *
		 $result_in = My_query("select *, part_comment as comment from parts where
		 $search_critiria
		 and status='in'
		 and owner in ( $user_restrict_subq )
		 order by $in_orderby");
		 */
		$result_in = My_query("
	      select
	       *,
	      parts.part_comment as comment,
	      parts.serial as serial
	      
	      from parts LEFT JOIN transaction ON
	      ( parts.serial = transaction.serial AND
	        parts.transaction_id = transaction.transaction_id )
	      where
	      $search_critiria and parts.status='in'
	      AND (parts.owner in ( $user_restrict_subq ) or parts.owner=?)
	      ",
	      $search_var, $username, $username, $username);

	      $in = $result_in->rowCount();
	} else {
		$result_in = NULL;
		$in = 0;
	}

	$result_out = My_query("
      select *, 
      parts.serial as serial,
	  DATE(ADDDATE(transaction.date, transaction.duebackin)) as duebackdate
      from parts LEFT JOIN ( transaction, customer ) ON
      ( parts.serial = transaction.serial AND
      transaction.customer_id = customer.customer_id AND
      parts.transaction_id = transaction.transaction_id )
      where
      $search_critiria and parts.status='out'
      AND (parts.owner in ( $user_restrict_subq ) or parts.owner=?)
      ",
      $search_var, $username, $username, $username);

      $out = $result_out->rowCount();

      if ($in + $out == 0) {
      	if ($info['serial']) {
      		$exist_check = My_query("select owner from parts where $search_critiria ", $search_var);
      		if ($exist_check->rowCount() == 0) {
      			if ($info['serial'] != ".") {
      				$info['err_message'] = "Cannot find parts with serial "
      				. strtoupper($info['serial']) .
	                        "<br/>You can use the following to add a new item into
	                        the database.";
      				print_new_checkin_form($info);
      				return;
      			} else {
      				$info['err_message'] = "You do not have anything in the database.";
      			}
      		} else {
      			$exist_line = $exist_check->fetch();
      			if ($info['serial'] != ".") {
      				$info['err_message'] = "Part with serial number $info[serial]
	                                    is owned by $exist_line[owner].";   
      				print_transact_form($info);
      				return;
      			} else {
      				$info['err_message'] = "You do not have anything in the database.";
      			}
      		}
      	} else if ($info['model']) {
      		$info['err_message'] = "Cannot find parts with model "
      		. strtoupper($info['model']);
      	} else if ($info['customer']) {
      		$info['err_message'] = ucwords($info['customer']) .
                                   " does not own any parts.";       
      	} else if ($info['company']) {
      		$info['err_message'] = ucwords($info['company']) .
                                   " does not own any parts.";
      	}
      	print_transact_form($info);
      	return;
      }

      // Printing results in HTML
      echo "<h3>Search Result</h3>";
      $msg = "Total " . ($in+$out). " items found, ";
      if ($in == 0)
      $msg .= "all are out.";
      if ($out == 0)
      $msg .= "all are in.";
      if ($in and $out)
      $msg .= "$in in and $out out.";
      print_info_msg($msg);
      $showswapincallout = 0;
      $showswapoutcallout = 0;
      if ($in) {

      	echo "<div><h4>In Stock</h4>\n";
      	echo "<table class='tablesorter' border='1' id='partintable'>";
      	print("<thead>");
      	if ($info['searchalldetail']) {
      		// echo "<tr><th colspan='4'>In Stock</th></tr>";
      		echo "<tr><th>SN</th><th>Model</th>
        <th>Comment</th>
        <th>Owner</th></tr>\n";
      	} else {
      		// echo "<tr><th colspan='3'>In Stock</th></tr>";
      		echo "<tr><th>SN</th><th>Model</th>";
      		echo "<th>Owner</th></tr>\n";
      	}
      	print("</thead><tbody>");
      	$rownumber=0;
      	while ($line = $result_in->fetch()) {
      		//print_r($line);
      		echo "<tr";
      		if ($rownumber%2==0) print(' class="even" >');
      		else print(' class="odd" >');
      		$rownumber++;
      		echo "<td>".
                    "<a href='$_SERVER[PHP_SELF]?".
                    "serial=$line[serial]&detail=1&timestamp=$MYTSTP'>";
      		echo strtoupper($line['serial']);
      		echo "</a>";
      		if ($line['action']=='request_checkout') {
      			print("*");
      		}
      		echo "</td>";
      		echo "<td>". strtoupper($line['model'])."</td>";
      		if ($info['searchalldetail']) {
      			echo "<td>$line[part_comment]</td>\n";
      		}
      		echo "<td>".$realname[$line['owner']]."</td>\n";
      		echo "</tr>\n";
      	}
      	print("</tbody>");
      	echo "</table>\n";
      	echo "</div>\n";
      }
      if ($out) {
      	echo "<div><h4>Out of Stock</h4>";
      	echo "<table class='tablesorter' border ='1' id='partouttable'>";
      	print("<thead>");
      	if ($info['searchalldetail']) {
      		//echo "<tr><th colspan='9'>Out of Stock</th></tr>";
      		echo "<tr><th>SN</th><th>Model</th><th>Customer</th>";
      		echo "<th>Company</th>";
      		echo "<th>Checked out by</th><th>Owner</th>";
      		echo "<th type='int'>Days Out</th><th type='date'>Expected Date</th><th>Comment</th>";
      	} else {
      		//echo "<tr><th colspan='5'>Out of Stock</th></tr>";
      		echo "<tr><th>SN</th><th>Model</th><th>Customer</th>";
      		echo "<th>Company</th>";
      		echo "<th>Owner</th>";
      	}
      	echo "</tr>\n";
      	print("</thead><tbody>");
      	$rownumber=0;
      	while ($line = $result_out->fetch()) {
      		
      		//print_r($line);
      		$short_customer_name = $line['customer_name'];
      		echo "<tr";
      		if ($rownumber%2==0) print(' class="even" >');
      		else print(' class="odd" >');
      		$rownumber++;
      		echo "<td>".
                    "<a href='$_SERVER[PHP_SELF]?".
                    "serial=$line[serial]&detail=1&timestamp=$MYTSTP'>";

      		echo strtoupper($line['serial']);
      		echo "</a>";
      		if ($line['action']=='request_checkin') {
      			print("*");
      			// get last transaction's details to display
      			$last_trans_ret = My_query("select * from transaction
      			      						where transaction_id = ?", $line['last_transaction_id']);
      			$last_trans = $last_trans_ret->fetch();
      			$line = array_merge($line, $last_trans);
      		}
      		echo "</td>";
      		echo "<td>". strtoupper($line['model'])."</td>";
      		echo "<td>". ucwords($short_customer_name)."</td>";
      		echo "<td>";
      		echo ucwords($line['company']);
      		echo "</td>\n";

      		if ($info['searchalldetail']) {
      			echo "<td>";
      			if (isset($realname[$line['username']])) echo $realname[$line['username']];
      			else echo $line['username'];
      			echo "</td>\n";
      		}
      		echo "<td>\n";
      		echo $realname[$line['owner']];
      		echo "</td>\n";
      		if ($info['searchalldetail']) {
      			/* Days out */
      			$daysout = My_query_ret("select DATEDIFF(CURDATE(), '$line[date]')");

      			/* due back date */
      			$duebackdate = My_query_ret("select DATE(ADDDATE('$line[date]', $line[duebackin]))");

      			/* Days remain */
      			$daysremain =My_query_ret("select DATEDIFF('$duebackdate', CURDATE())");

      			echo "<td>";
      			echo $daysout;
      			echo "</td>";
      			echo "<td>";
      			if ($daysremain <=0)
      			print_error_msg($duebackdate);
      			else
      			print($duebackdate);
      			echo "</td>";
      			echo "<td>";
      			echo $line['transaction_comment'];
      			echo "</td>\n";
      		}
      		echo "</tr>\n";
      	}
      	print("</tbody>");
      	echo "</table>\n";
      	echo "</div>";
      }
      //echo "<br/>";
      if ($showswapoutcallout) {
      	echo "<a name = 'swapout'> * Indicate this is a swapped out part</a><br />";
      }
      if ($showswapincallout) {
      	echo "<a name = 'swapin'> ** Indicate this is a swapped in part</a><br />";
      }
      echo "</p><div class='saperator'></div>";

      //
      // Free resultset
      //if ($result_in != null) mysql_free_result($result_in);
      //if ($result_out != null) mysql_free_result($result_out);
      print_transact_form($info);
}

function print_login_form() {
	print_page_title();
    print <<<END
<div class="container">

      <form class="form-signin" action='./index.php' method='post'>
        <h2 class="form-signin-heading">Please log in</h2>
        <input name='username' type="text" class="form-control" placeholder="User name" autofocus>
        <input name='password' type="password" class="form-control" placeholder="Password">
<label class="checkbox">
          <input type="checkbox" value="remember-me"> Remember me
        </label>
        <button name='login' class="btn btn-block btn-primary" type="submit">Log in</button>
      </form>

END;
	if (isset($login_err)) echo "$login_err";
	print("</div> <!-- /container -->");
}

/**
 *
 */
function get_info() {
	global $get_info_done;
	if ($get_info_done) return;
	$get_info_done = TRUE;


	$serial = isset($_REQUEST['serial']) ? escape_string($_REQUEST['serial']) : '';
	$comment = isset($_POST['comment']) ? escape_string($_POST['comment'], 1) : '';
	$trans_comment = isset($_POST['trans_comment']) ? escape_string($_POST['trans_comment'], 1) : '';
	$model = isset($_REQUEST['model']) ? escape_string($_REQUEST['model']) : '';
	$customer = isset($_POST['customer']) ? escape_string($_POST['customer']) : '';
	$duebackin = isset($_POST['duebackin']) ? escape_string($_POST['duebackin']) : '';
	$orderby = isset($_POST['orderby']) ? escape_string($_POST['orderby']) : '';

	if (! is_numeric($duebackin) ) $duebackin = '';

	$searchstr = isset($_POST['searchstr'])?escape_string($_POST['searchstr']):'';
	$searchalldetail = isset($_REQUEST['searchalldetail'])?escape_string($_REQUEST['searchalldetail']):'';
	$company = '';
	$owner = isset($_POST['owner']) ? escape_string($_POST['owner']) : '';

	$old_serial = isset($_REQUEST['old_serial'])?escape_string($_REQUEST['old_serial']):"";
	$old_comment = isset($_POST['old_comment'])?escape_string($_POST['old_comment'], 1):"";
	$old_trans_comment = isset($_POST['old_trans_comment'])?escape_string($_POST['old_trans_comment'], 1):"";
	$old_model = isset($_REQUEST['old_model'])?escape_string($_REQUEST['old_model']):"";
	$old_owner = isset($_POST['old_owner']) ? escape_string($_POST['old_owner']) : '';

	$company = isset($_POST['company'])?escape_string($_POST['company'], 1):"";

	if ($searchalldetail) {
		$model = ".*";
	}

	$ret = array('serial'=>$serial,
            'comment'=>$comment,
            'trans_comment'=>$trans_comment,
            'model'=>$model,
            'customer'=>$customer,
            'company'=>$company,
            'duebackin'=>$duebackin,
            'searchstr'=>$searchstr,
            'searchalldetail'=>$searchalldetail,
            'owner'=>$owner,
            'old_serial'=>$old_serial,
            'old_model'=>$old_model,
            'old_comment'=>$old_comment,
            'old_trans_comment'=>$old_trans_comment,
            'old_owner'=>$old_owner,
            'orderby'=>$orderby);


	if (isset($_SESSION["username"])) {
		$q = "select username from user where username=?";
		$result = My_query($q, $_SESSION["username"]);
		if ($result->rowCount()==1) {
			$line = $result->fetch();
			$ret['username'] = $line['username'];
		}
	}

	return $ret;

}


function transact($info = array(),  $request, $reprint=1) {
	/*
	 * transact is a consolidated function to handle both check in and check out a part.
	 *
	 * $request: checkin or checkout
	 */
	global $realname;
	$mysql_date = date("Y-m-d H:i:s");

	$serial = $info['serial'];
	if (strlen($serial) == 0) {
		$info['err_message'] = "Need serial number.";
		if ($reprint) print_transact_form($info);
		return FALSE;
	}
	$comment = $info['comment'];
	$custmer_id = 0;
	$username=$_SESSION['username'];
	$user_email=My_query_ret("select email from user where username=?", $_SESSION['username']);

	/*print("<pre>info = ");
	 print_r($info);
	 print("</pre>");*/

	db_lock_all_tables();
	$sql_res = My_query("select * from
						 parts JOIN transaction 
						 ON parts.transaction_id = transaction.transaction_id
						 where parts.serial=?", $serial);
	if ($sql_res->rowCount() == 0) {
		$info['err_message'] = "Cannot find part with serial number ".
		strtoupper($serial) .
				 ".<br/>You can use the following to add a new item into the database.";
		if ($reprint) print_new_checkin_form($info);
		My_query("UNLOCK TABLES");
		return FALSE;
	}

	$part_array = $sql_res->fetch();
	$part_status = $part_array['status'];
	$model = strtoupper($part_array['model']);
	$transaction_action = $part_array['action'];
	$transaction_username=$part_array['username'];
	$old_transaction_id=$part_array['transaction_id'];
	$old_customer_id=$part_array['customer_id'];

	if ($request=="checkout" and $part_status == 'out') {
		$info['err_message'] = "$model with serial number ".
		strtoupper($serial). " is out of stock";
		if ($reprint) print_transact_form($info);
		My_query("UNLOCK TABLES");
		return FALSE;
	} else if ($request=="checkin" and $part_status=='in') {
		$info['err_message'] = "$model with serial number ".
		strtoupper($serial). " is in stock. No checking in needed";
		if ($reprint) print_transact_form($info);
		My_query("UNLOCK TABLES");
		return FALSE;
	} else if ($transaction_action=="request_checkout" or $transaction_action=="request_checkin") {
		$info['err_message'] = "$model with serial number ".
		strtoupper($serial). " has been put on hold by a request from ".
		$realname[$transaction_username].
				 "<br/>".
				 "Click <a href='./index.php?approve=1&serialarray[]=$serial'>here</a> to approve or deny the request before a new
				 transaction can start.";
		if ($reprint) print_transact_form($info);
		My_query("UNLOCK TABLES");
		return FALSE;
	}

	$permit = check_permission($_SESSION['username'], $serial);

	if ($permit['permission'] == RIGHT_RO) {
		$info['err_message'] = "$model with serial number ".
		strtoupper($serial). " does not belongs to you.
				 An email has been sent out for approval. Once approved, the
				 transaction can continue.";
		$transaction_action = "request_".$request;
	} else if ($permit['permission'] == RIGHT_FULL) {
		if ($request=="checkin") $transaction_action = "in";
		else if ($request=="checkout") $transaction_action = "out";
		else {
			print_error_msg("<b>request is not checkin or checkout.</b>");
			My_query("UNLOCK TABLES");
			return FALSE;
		}
	} else {
		$info['err_message'] = "You do not have right to ".$request." this item.";
		if ($reprint) print_transact_form($info);
		My_query("UNLOCK TABLES");
		return FALSE;
	}


	if ($request=="checkout") {
		$customer = $info['customer'];
			
		$company = $info['company'];

		if ($customer=='' || $company=='') {
			$info['err_message'] = "Customer and company
										 name must be filled in.";
			if ($reprint) print_transact_form($info);
			My_query("UNLOCK TABLES");
			return FALSE;
		}


		$duebackin = $info['duebackin'];
		if (!is_numeric($duebackin)) {
			$info['err_message'] = "Need a due back time (in days) to check out an item.";
			if ($reprint) print_transact_form($info);
			My_query("UNLOCK TABLES");
			return FALSE;
		}
		$value = trim($comment);
		if (empty($value)) {
			$info['err_message'] = "Please provide a reason for check out in comment.";
			if ($reprint) print_transact_form($info);
			My_query("UNLOCK TABLES");
			return FALSE;
		}

		// update part, customerm, and transactions
		$sql_res = My_query("select customer_id from customer
					   where customer_name=? and company=?", $customer, $company);
		if ($sql_res->rowCount() == 0) {
			// new customer, add
			My_query("insert customer
				set customer_name=?, company=?", $customer, $company );
		}
		$sql_res = My_query("select customer_id from customer
				   where customer_name=? and company=?", $customer, $company);
		$record = $sql_res->fetch();
		$customer_id = $record['customer_id'];

		My_query("insert transaction set
					serial=?,
					action=?,
					customer_id=?,
					username=?,
					duebackin=$duebackin,
				transaction_comment=?,
				last_transaction_id=$old_transaction_id",
		$serial, $transaction_action, $customer_id, $_SESSION['username'], $comment);
	}
	if ($request=="checkin") {
		if ($transaction_action=='request_checkin') {
			My_query("insert transaction
					 set serial=?, 
					 action=?, 
					 username=?,
					 transaction_comment=?,
					 customer_id=$old_customer_id,
					 last_transaction_id=$old_transaction_id",
			$serial, $transaction_action, $_SESSION['username'], $comment);
		} else {
			My_query("insert transaction
					 set serial=?, 
					 action=?, 
					 username=?,
					 transaction_comment=?,
					 last_transaction_id=$old_transaction_id",
			$serial, $transaction_action, $_SESSION['username'], $comment);
		}
	}

	$result=My_query("select last_insert_id()");
	$result_array = $result->fetch();
	$transaction_id = $result_array[0];

	if ($transaction_action=='out') {
		My_query("update parts set
				 status='out', transaction_id=?
					 where serial=?", $transaction_id, $serial);	
		$info['message'] = "$model with serial ". strtoupper($serial).
			 " is checked out to ".ucwords($customer)." of ".
		ucwords($company) . ".";
	} else if ($transaction_action=='in') {
		My_query("update parts set
				 status='in', transaction_id=?
					 where serial=?", $transaction_id, $serial);	
		$info['message'] = "$model with serial ". strtoupper($serial).
			 " is checked in.";
	} else if ($transaction_action=='request_checkout') {
		My_query("update parts set
				 transaction_id=?
					 where serial=?", $transaction_id, $serial);
		
		//print_r($permit);
		
		foreach ($permit['approvals'] as $admin_user) {
			$result=My_query("select * from user where username=?", $admin_user);
			$line=$result->fetch();


			$to      = $line['email'];
			$subject = 'Inv DB Checkout Request';
			$message = "
Hi, 
			 
I would like to checkout the following item from the inventory database.
			 
 Serial:   ".strtoupper($serial)."
 Model:    ".strtoupper($model)."
 Customer: ".ucwords($customer)."
 Compnay:  ".ucwords($company)."
			 
 Dueback:  $duebackin days
			 
 Comment:  $comment

 
Regards,
			$realname[$username]
";

			$header = "Reply-To: $realname[$username] <$user_email>\r\n";
			$header .= "Return-Path: $realname[$username] <$user_email>\r\n";
			$header .= "From: $realname[$username] <$user_email>\r\n";
			$header .= "Organization: Ixia Communications\r\n";
			$header .= "Content-Type: text/plain\r\n";

			mail($to, $subject, $message, $header);
		}
		$info['message'] = "$model with serial ". strtoupper($serial).
			 " is requested to be checked out to ".ucwords($customer)." of ".
		ucwords($company) . ". A request email has been sent to ". implode(", ", $permit['approvals']);
	} else if ($transaction_action=='request_checkin') {
		My_query("update parts set
				 transaction_id=?
					 where serial=?",  $transaction_id, $serial);
		foreach ($permit['approvals'] as $admin_user) {
			$result=My_query("select * from user where username=?", $admin_user);
			$line=$result->fetch();


			$to      = $line['email'];
			$subject = 'Inv DB Checkin Request';
			$message = "
Hi, 
			 
I would like to checkin the following item into the inventory database.
			 
 Serial:   ".strtoupper($serial)."
 Model:    ".strtoupper($model)."
			 
Comment:  $comment
	 
			 
Regards,
			$realname[$username]
";

			$header = "Reply-To: $realname[$username] <$user_email>\r\n";
			$header .= "Return-Path: $realname[$username] <$user_email>\r\n";
			$header .= "From: $realname[$username] <$user_email>\r\n";
			$header .= "Organization: Ixia Communications\r\n";
			$header .= "Content-Type: text/plain\r\n";

			mail($to, $subject, $message, $header);
		}
		$info['message'] = "$model with serial ". strtoupper($serial).
			 " is requested to be checked in. A request email has been sent to ". 
		implode(", ", $permit['approvals']);
	}
	My_query("UNLOCK TABLES");

	if ($reprint)  print_transact_form($info);
	return TRUE;
}

function count_approval() {
	$username=$_SESSION['username'];
	$count=0;
	db_lock_all_tables();
	$result = My_query("select *
		from parts JOIN ( transaction, customer ) ON
	      ( parts.serial = transaction.serial AND
	      transaction.customer_id = customer.customer_id AND
	      parts.transaction_id = transaction.transaction_id )
	      where transaction.action='request_checkout' or
	           transaction.action='request_checkin'
	");
	while($line=$result->fetch()) {
		$permit=check_permission($username, $line['serial']);
		if ($permit['permission']==RIGHT_FULL) {
			$count++;
		}
	}
	My_query("UNLOCK TABLES");
	return $count;
}

function approve_request($serial) {
	$username=$_SESSION['username'];
	$permit=check_permission($username, $serial);
	if ($permit['permission']!=RIGHT_FULL) {
		return FAIL;
	}
	db_lock_all_tables();
	$result = My_query("select *
		from parts JOIN ( transaction, customer ) ON
	      ( parts.serial = transaction.serial AND
	      transaction.customer_id = customer.customer_id AND
	      parts.transaction_id = transaction.transaction_id )
	      where transaction.action='request_checkout' or
	           transaction.action='request_checkin' and parts.serial = ?
	", $serial);
	if ($result->rowCount()<1) {
		My_query("UNLOCK TABLES");
		return FAIL;
	}
	$line=$result->fetch();
	if ($line['action']=='request_checkout') {
		My_query("insert transaction set
							serial=?,
							action='approve',
							customer_id=?,
							username=?,
							duebackin=$line[duebackin],
							transaction_comment=?",
		$line['serial'], $line['customer_id'], $username,
						"APPROVED BY $username");
		//check it out
		My_query("insert transaction set
							serial=?,
							action='out',
							customer_id=?,
							username=?,
							duebackin=$line[duebackin],
							transaction_comment=?",
		$line['serial'], $line['customer_id'], $line['username'],
		$line['transaction_comment']);

		$transaction_id=My_query_ret("select last_insert_id()");

		My_query("update parts set
				 			status='out', transaction_id='$transaction_id'
					 		where serial='$line[serial]'");
		#FIXME enhance email notification
		$ret=PASS;
	} else if ($line['action']=='request_checkin') {
		My_query("insert transaction set
							serial=?,
							action='approve',
							username=?,
							transaction_comment=?",
		$line['serial'], $username,
						"APPROVED BY $username");
		// check int
		My_query("insert transaction set
							serial=?,
							action='in',
							username=?",
		$line['serial'], $line['username']);

		$transaction_id=My_query_ret("select last_insert_id()");
		My_query("update parts set
				 			status='in', transaction_id='$transaction_id'
					 		where serial=?", $line['serial']);
		$ret=PASS;
	} else {
		$ret= FAIL;
	}
	My_query("UNLOCK TABLES");
	return $ret;
}
function deny_request($serial) {
	$username=$_SESSION['username'];
	$permit=check_permission($username, $serial);
	if ($permit['permission']!=RIGHT_FULL) {
		return FAIL;
	}
	db_lock_all_tables();
	$result = My_query("select *
		from parts JOIN ( transaction, customer ) ON
	      ( parts.serial = transaction.serial AND
	      transaction.customer_id = customer.customer_id AND
	      parts.transaction_id = transaction.transaction_id )
	      where transaction.action='request_checkout' or
	           transaction.action='request_checkin' and parts.serial = ?
	", $serial);
	if ($result->rowCount()<1) {
		My_query("UNLOCK TABLES");
		return FAIL;
	}
	$line=$result->fetch();
	if ($line['action']=='request_checkout') {
		My_query("insert transaction set
							serial=?,
							action='deny',
							customer_id=?,
							username=?,
							duebackin=$line[duebackin],
							transaction_comment=?",
		$line['serial'], $line['customer_id'], $username,
						"DENIED BY $username");


		My_query("update parts set
				 			transaction_id=?
					 		where serial=?",
		$line['last_transaction_id'], $serial);

		#FIXME enhance email notification
		$ret=PASS;
	} else if ($line['action']=='request_checkin') {
		My_query("insert transaction set
							serial=?,
							action='deny',
							username=?,
							transaction_comment=?",
		$line['serial'], $username,
						"DENIED BY $username");

		My_query("update parts set
				 			transaction_id=?
					 		where serial=?",
		$line['last_transaction_id'], $serial);
		$ret=PASS;
	} else {
		$ret= FAIL;
	}
	My_query("UNLOCK TABLES");
	return $ret;
}

function print_approval_form($info) {
	global $realname;
	$mysql_date = date("Y-m-d H:i:s");

	$username=$_SESSION['username'];

	db_lock_all_tables();
	$result = My_query("select *
		from parts JOIN ( transaction, customer ) ON
	      ( parts.serial = transaction.serial AND
	      transaction.customer_id = customer.customer_id AND
	      parts.transaction_id = transaction.transaction_id )
	      where transaction.action='request_checkout' or
	           transaction.action='request_checkin'
	");

	$count=0;

	while($line=$result->fetch()) {
		$permit=check_permission($username, $line['serial']);
		if ($permit['permission']==RIGHT_FULL) {
			$count++;
			if ($count==1) {
				print("<h3>The following item(s) need your approval to be checked in/out.</h3>");
				print("<table id='approvaltable' border='1'>\n");
				print("<tr><th>User</th>");
				print("<th>Request</th>");
				print("<th>Serial</th>");
				print("<th>Model</th>");
				print("<th>Customer</th>");
				print("<th>Date</th>");
				print("<th>Time (Days)</th>");
				print("<th>Comment</th>");
				print("<th>Approve</th><th>Deny</th>");
			}
			print("<tr>");
			print("<td>".$realname[$line['username']]."</td>");
			if ($line['action'] == 'request_checkout') {
				print("<td>Loan</td>");
			} else {
				print("<td>Return</td>");
			}
			print("<td>".strtoupper($line['serial'])."</td>");
			print("<td>".strtoupper($line['model'])."</td>");
			print("<td>".ucwords($line['customer_name']).
					", ".ucwords($line['company'])."</td>");
			print("<td>$line[date]</td>");
			print("<td>$line[duebackin]</td>");
			print("<td>$line[transaction_comment]</td>");
			print("<td><button type='button' class='approve_request' value='$line[serial]'>Approve</button></td>");
			print("<td><button type='button' class='deny_request' value='$line[serial]'>Deny</button></td>");
			print("</tr>");
		}
	}

	if ($count!=0) {
		print("</table>");
	} else {
		print_info_msg("No request.");
	}

	My_query("UNLOCK TABLES");
}

function newcheckin($info=array(), $reprint=1) {
	$serial = $info['serial'];
	if ( strlen($serial) == 0 ) {
		$info['err_message'] = "Need serial number.";
		if ($reprint) print_new_checkin_form($info);
		return FALSE;
	}
	$sql_res = My_query("select model, serial,status
	                         from parts where serial=?", $serial);
	if ($sql_res->rowCount() != 0) {
		// existing part
		$info['err_message'] = "Parts with $serial already exists";
		if ($reprint) print_new_checkin_form($info);
		return FALSE;
	}

	// update part table
	/* Make sure Owner exists */
	/*print_r($info);*/
	$mysql_date = date("Y-m-d H:i:s");

	$q = "select username from user
    where isowner=1 and username=?";
	$result = My_query($q, $info['owner']);
	if ( $result->rowCount() == 0) {
		$info['err_message'] = "User $info[owner] cannot own a part.";
		if ($reprint) print_new_checkin_form($info);
		return FALSE;
	}
	if (!$info['model']) {
		$info['err_message'] = "Please specify the model name.";
		if ($reprint) print_new_checkin_form($info);
		return FALSE;
	}
	// check owner
	$users=get_same_group_users($_SESSION['username']);

	$write_ok = 0;
	foreach ($users as $who_can_write) {
		if ($info['owner'] == $who_can_write) {
			$write_ok = 1;
		}
	}
	if (!$write_ok) {
		$info['err_message'] = "Try to check in new part not owned by people in your group. Failed.";
		if ($reprint) print_transact_form($info);
		return FALSE;
	}

	db_lock_all_tables();

	if (isset($_POST['comment'])) {
		My_query("insert transaction set serial=?,
            action='in', username=?,
			transaction_comment=?", $info['serial'], $_SESSION['username'], $info['comment']);
	} else {
		My_query("insert transaction set serial=?,
            action='in', username=?", $info['serial'], $_SESSION['username']);
	}


	$result=My_query("select last_insert_id()");
	$result_array = $result->fetch();
	$transaction_id = $result_array[0];
	$q = "insert parts set serial=?, model=?,
  status='in', transaction_id=?, owner=?,
  part_comment=?";
	My_query($q, $info['serial'], $info['model'], $transaction_id, $info['owner'], $info['comment'] );

	My_query("UNLOCK TABLES");
	$info['message'] = strtoupper($info['model']).
            " with serial ". strtoupper($info['serial']).
            " is checked in successfully.";
	if ($reprint)  print_transact_form($info);
	return TRUE;
}

function showdetail($info=array()) {
	global $realname;
	if (!$info['serial']) {
		print_transact_form();
		return;
	}
	$sql_res = My_query("select * from parts where serial=?", $info['serial']);
	if ($sql_res->rowCount() == 0) {
		$info['err_message'] = "Cannot find part with serial number
		$info[serial]!" .
                "<br/>You can use the following to add a new item into the database.";
		print_new_checkin_form($info);
		return;
	}
	$result_array = $sql_res->fetch();
	$serial = strtoupper($result_array['serial']);
	$model = strtoupper($result_array['model']);
	$status = $result_array['status'];
	$transaction_id = $result_array['transaction_id'];
	$part_comment = $result_array['part_comment'];
	$owner = $realname[$result_array['owner']];

	$sql_res = My_query("select * from transaction
	                         where transaction_id=?", $transaction_id);
	$result_array = $sql_res->fetch();
	$customer_id = $result_array['customer_id'];
	if (isset($realname[$result_array['username']])) {
		$username = $realname[$result_array['username']];
	} else {
		$username = $result_array['username'];
	}
	$date = $result_array['date'];
	$duebackin = $result_array['duebackin'];
	$tran_comment = $result_array['transaction_comment'];

	echo "<h3>Part Detail Information</h3>\n";
	echo "<p>Serial: <b>$serial</b><br />Model:
	<b>$model</b><br />Owner: <b>$owner</b><br />Status: <b>$status</b>";
	if ($part_comment) echo "<br />Comment: $part_comment";

	/*echo "<h3>Last Transaction</h3>\n";
	echo "<p>Checked $status by <b>$username</b> on <b>$date</b>";
	if ($status == "out") {
		$sql_res = My_query("select * from customer
	  		where customer_id=?", $customer_id);
		$result_array = $sql_res->fetch();
		$customer_name = ucwords($result_array['customer_name']);
		$company = ucwords($result_array['company']);
		echo ", loaned to <b>$customer_name</b> in <b>$company</b>";
		if ($duebackin) {
			echo "<br />Due back on ";
			$duebackdate = My_query_ret("select DATE(ADDDATE('$date', $duebackin))");
			echo $duebackdate;
			$daysremain = My_query_ret("select DATEDIFF('$duebackdate', CURDATE())");
			if ($daysremain < 0) {
				$overduedays = -$daysremain;
				print_error_msg("Item is over due by $overduedays days!");
			}
		}
	}
	echo ".";
	if ($tran_comment) echo "<br />Comment: $tran_comment";
	*/
	echo "</p><div class='saperator'></div>";
	echo "<a href='$_SERVER[PHP_SELF]?serial=$serial&history=1'>
	Show History</a>";
	echo " | <a href='$_SERVER[PHP_SELF]?serial=$serial&editpart=1'>Edit Part</a>";
	echo "<div class='saperator'></div>";
	print_transact_form($info);

}

function showhistory($info=array()) {
	global $realname;
	if (!$info['serial']) {
		print_transact_form();
		exit();
	}
	$sql_res = My_query("select * from parts where serial=?", $info['serial']);
	if ($sql_res->rowCount() == 0) {
		$info['err_message'] = "Cannot find part with serial number
		$info[serial]!" .
                "<br/>You can use the following to add a new item into the database.";
		print_transact_form($info);
		exit();
	}
	$result_array = $sql_res->fetch();
	$serial = strtoupper($result_array['serial']);
	$model = strtoupper($result_array['model']);
	$status = $result_array['status'];
	$comment = $result_array['part_comment'];
	$owner = $realname[$result_array['owner']];


	echo "<p>Serial: <b>$serial</b><br />Model: <b>$model</b><br />
               Owner: <b>$owner</b><br />Status: <b>$status</b><br />";
	if ($comment) echo "Comment: $comment<br />";


	echo "</p><div class='saperator'></div>";

	$sql_res = My_query("select * from transaction
	where serial='$info[serial]' order by date");


	while ($line = $sql_res->fetch()) {

		$customer_id = $line['customer_id'];
		if (isset($realname[$line['username']])) {
			$username = $realname[$line['username']];
		} else {
			$username = $line['username'];
		}
		$date = $line['date'];
		$tran_comment = $line['transaction_comment'];
		$action = $line['action'];

		echo "<p>Checked <b>$action</b> by <b>$username</b> on <b>$date</b>";
		if ($action == "out") {
			$result = My_query("select * from customer
	    where customer_id='$customer_id'");
			$result_array = $result->fetch();
			$customer_name = ucwords($result_array['customer_name']);
			$company = ucwords($result_array['company']);
			echo ", loaned to <b>$customer_name</b> in <b>$company</b>";
			echo ". <br />Due back in $line[duebackin] days";
		}
		echo ".";
		if ($tran_comment) echo "<br />Comment: $tran_comment";

		echo "</p><div class='saperator'></div>";
	}
	echo "<a href='$_SERVER[PHP_SELF]?serial=$serial&editpart=1'>Edit Part</a>";
	echo "<div class='saperator'></div>";
	print_transact_form($info);
}

function print_upload_form() {
	echo "<h3>Upload a CSV file</h3>";
	echo "<p>Create an Exel file with columns in the following order:</p>";
	echo "<p>";
	echo "<ol>";
	echo "<li><b>serial</b>: Serial Number</li>";
	echo "<li><b>model</b>: Model name, e.g. LM1000STXS4-256</li>";
	echo "<li><b>comment</b>: Part comment, free format</li>";
	echo "<li><b>status</b>: Current status, in or out</li>";
	echo "<li><b>customer name</b>: When status is out, who has it?</li>";
	echo "<li><b>company</b>: When status is out, where is it?</li>";
	echo "<li><b>owner</b>: Who owns this item.</li>";
	echo "</ol></p>";
	echo "<p>Save the Exel file in .csv format and upload it here. They will be";
	echo " automatically imported in to the database under your username.</p>";
	echo "</p>";
	echo "<form enctype='multipart/form-data' action='./index.php'
         method='POST'>
    <!-- MAX_FILE_SIZE must precede the file input field -->
    <input type='hidden' name='MAX_FILE_SIZE' value='300000' />
    <!-- Name of input element determines name in $_FILES array -->
    Select a CSV file: <input name='userfile' type='file' /><br />
    <input type='submit' name='processuploadfile' value='Upload' />
    </form>";
}

function process_upload_file() {
	$uploaddir = sys_get_temp_dir();
	$uploadfile = $uploaddir ."/". basename($_FILES['userfile']['name']);
	if (move_uploaded_file($_FILES['userfile']['tmp_name'], $uploadfile)) {
		$row = 1;
		$title=array('serial', 'model', 'part_comment','status','customer_name',
                'company', 'owner');
		$handle = fopen($uploadfile, "r");
		//discard the first line
		$data = fgetcsv($handle, 1000, ",");
		if ($data[0]!='serial' || count($data) < count($title)) {
			echo "<p>The first row needs to be header. Or format is not correct.</p>";
			print_upload_form();
			return;
		}
		while (($data = fgetcsv($handle, 1000, ",")) !== FALSE) {
			if (count($data) < count($title)) continue;
			$info = array("serial"=>$data[0] ? escape_string($data[0]) : '',
                    "model"=>$data[1] ? escape_string($data[1]) : '',
                    "comment"=>$data[2] ? escape_string($data[2], 1) : '',
                    "status"=>$data[3] ? escape_string($data[3]) : '',
                    "customer"=>$data[4] ? escape_string($data[4], 1) : '',
                    "company"=>$data[5] ? escape_string($data[5], 1) : '',
                    "owner"=>$data[6] ? escape_string($data[6]) : '',
                    "duebackin"=>'30');
			if ($info['serial'] == '' || $info['model'] == '') {
				print_error_msg("ERROR: serial and model cannot be empty. ");
				continue;
			}

			if ($info['status'] != 'out' && $info['status'] != 'in' ) {
				print_error_msg("ERROR: $info[serial] status must be in or out");
				continue;
			}

			if ($info['status'] =='out' && ($info['customer'] == '' ||
			$info['company'] == '')) {
				print_error_msg("ERROR: $info[serial] needs customer name and
                      company name.");
				continue;
			}

			if ($info['owner'] == '') {
				print_error_msg("ERROR: $info[serial] owner cannot be empty.");
				continue;
			}

			$q = "select username from user
          where isowner=1 and username=?";
			$result = My_query($q, $info['owner']);
			if ( $result->rowCount() == 0) {
				$q1 = "select username from user where isowner=1 and
               realname=?";
				$result1 = My_query($q1, $info['owner']);
				if ( $result1->rowCount() == 0) {
					print_error_msg("ERROR: $info[serial]: $info[owner] is not a valid
                         owner.");
					continue;
				} else {
					$line = $result1->fetch();
					$info['owner'] = $line['username'];

				}
			}

			if (newcheckin($info, 0)) {
				print_info_msg("$info[serial] is checked in successfully.");
			} else {
				print_info_msg("$info[serial] failed to add to database.");
				if (isset($info['err_message'])) {
					print_error_msg("$info[err_message]");
				}
				print ("<br/>");
				continue;
			}

			if ($info['status'] == 'out') {
				if (transact($info, 'checkout', 0)) {
					print_info_msg("$info[serial] successfully checked out to
					$info[customer] in $info[company].");
				} else {
					print_error_msg("$info[serial] failed to checkout.");
					if (isset($info['err_message'])) {
						print_error_msg("$info[err_message]");
					}
					print ("<br/>");
				}
			}
			print("<br />");
		}
		fclose($handle);
	} else {
		echo "File upload failed.";
	}

}

function print_manage_form($info) {
	/*
	 * TODO:
	 * Superuser Manage users and groups
	 *
	 * o) create group
	 * o) move users in/out of groups
	 * o) assign group admin
	 * o) delete group
	 * o) delete user
	 *
	 */
	session_commit();
	//print_r ($_SESSION);
	if (isset($_SESSION['username']) && $_SESSION['username']!='') {
		$result = My_query("select * from user where username=? and issuperuser=1", $_SESSION['username']);
		if ($result->rowCount()<1) {
			echo "You do not have the right to manage user and groups.";
			return;
		}
	} else {
		echo "Need login";
		return;
	}
	echo "<h3>Under construction</h3>";

	echo "<h3>User and Group Management</h3>";

	print <<<END
	<div id="creategroup">
	<form name="creategroupform">
	<input type="hidden" name="manage" value="1"/>
	<input type="text" name="groupname" />
	<input type="submit" name="creategroup" value="Create A Group"/>
	</form>
	</div>
END;
	if (isset($_REQUEST['creategroup']) && isset($_REQUEST['groupname'])) {
		$groupname=escape_string($_REQUEST['groupname'],1);
		$ret = add_group($groupname);
		if ($ret['result']==PASS) {
			print_info_msg("<div>Group $groupname is created successfully.</div>");
		} else {
			print_info_msg("<div>Group $groupname failed to be created. $ret[message]</div>");
		}
	}

	print <<<END
	<div id="creategroup">
	<form name="creategroupform">
	<input type="hidden" name="manage" value="1"/>
	<input type="text" name="groupname" />
	<input type="submit" name="creategroup" value="Create A Group"/>
	</form>
	</div>
END;
}

function get_all_users($group="") {
	if ($group=='')	 {
		$result=My_query("select distinct user from user_group_mapping");
	} else {
		$result=My_query("select distinct user from user_group_mapping where `group`=?", $group);
	}
	$ret = array();
	while($line=$result->fetch()) {
		$ret[]=$line['user'];
	}
	return $ret;
}

function get_all_groups($user="") {
	if ($user=='')	 {
		$result=My_query("select distinct `group` from user_group_mapping");
	} else {
		$result=My_query("select distinct `group` from user_group_mapping where `user`=?", $user);
	}
	$ret = array();
	while($line=$result->fetch()) {
		$ret[]=$line['group'];
	}
	return $ret;
}



function clear_session() {
	// Unset all of the session variables.
	$_SESSION = array();

	// If it's desired to kill the session, also delete the session cookie.
	// Note: This will destroy the session, and not just the session data!
	if (ini_get("session.use_cookies")) {
		$params = session_get_cookie_params();
		setcookie(session_name(), '', time() - 42000,
		$params["path"], $params["domain"],
		$params["secure"], $params["httponly"]
		);
	}

	// Finally, destroy the session.
	session_destroy();
}

function debug_print($v) {
	if (DEBUG) var_dump($v);
}

function print_html_header($page) {
	?>

  <!DOCTYPE HTML>
  <html>
  <head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <meta name="keywords" content="Inventory Database" />
  
  <link rel="shortcut icon" href="img/favicon.ico" />
  <title><?php echo $page;?></title>
  
  
  <!-- jQuery (necessary for Bootstrap's JavaScript plugins) -->
  <script src="https://code.jquery.com/jquery-1.11.1.min.js"></script>
  <script src="https://code.jquery.com/ui/1.11.1/jquery-ui.min.js"></script>
  
  <script type='text/javascript' src='js/invlib.js'></script>
  
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <!-- Bootstrap -->
  <!-- Latest compiled and minified CSS -->
  <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/css/bootstrap.min.css">
  <!-- Latest compiled and minified JavaScript -->
  <script src="https://maxcdn.bootstrapcdn.com/bootstrap/3.2.0/js/bootstrap.min.js"></script>

  <link type="text/css" href="css/invstyle.css" rel="Stylesheet" />

	<?php
	switch ($page) {
		case HOMEPAGE:
		case CHANGEPROFILEPAGE:
		case HELP:
			print <<<END
		<script type='text/javascript' src='js/homepage.js'></script>
		<script type="text/javascript" src="jq/tablesorter/jquery.tablesorter.min.js"></script>
		<link type="text/css" href="./jq/tablesorter/themes/blue/style.css" rel="Stylesheet" />
		
END;
			break;
		case MANAGEPAGE:
			print <<<END
		<script type='text/javascript' src='js/manage.js'></script>
		 
END;
			break;
		default:
			break;
	}

	?>


</head>
<body>

<?php

}

?>
