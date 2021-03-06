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
include "lib/lib2.inc";

session_start();
db_init(null);
$result=authenticate();
session_commit();

?>

<?php 
print_html_header(HOMEPAGE);


if($result!=PASS) {
	/* Not logged in */
	
	print_login_form();
	
} else {

	$info = get_info();

	// Now start main proc and print the real page

	/* logged in successfully */
	print_page_title();
	print("<div id='message'></div>");
	print("<div id='main'>");
	/*
	 * The $_REQUEST of a web page should be structual. In this case, if redesigned,
	 * it should take an arg called action, then do a switch on the action to determine
	 * which function to call in the following code.
	 * Each function complete one action. This can be think of each action is actually
	 * a relatively separated page in the customer point of view.
	 * Inside that function, it will examing more $_REQUEST variable to make sub-actions
	 * on what exactly need to be done.
	 * E.G. if action==manage, then call action_manage. In action_manage, check
	 * if it is a create, delete, move, add, etc. by checking the value of
	 * $_REQUEST['manage_sub_action'].
	 */
	if (isset($_POST['checkin'])) {
		//checkin($info);
		transact($info, "checkin");
	} else if (isset($_POST['checkout'])) {
		//checkout($info);
		transact($info, "checkout");
	} else if (isset($_POST['new_checkin'])) {
		newcheckin($info);
	} else if (isset($_POST['search'])) {
		print_search_result($info);
	} else if (isset($_POST['cancel'])) {
		print_transact_form();
	} else if (isset($_REQUEST['searchall'])) {
		$info['serial']='.';
		print_search_result($info);
	} else if (isset($_REQUEST['searchalldetail'])) {
		$info['serial']='.';
		print_search_result($info);
	} else if (isset($_GET['detail']) || isset($_REQUEST['checkupdatepart'])) {
		showdetail($info);
	} else if (isset($_GET['history'])) {
		showhistory($info);
	} else if (isset($_GET['editpart'])) {
		print_edit_form($info);
	} else if (isset($_POST['updatepart'])) {
		updatepart($info);
	} else if (isset($_POST['deletepart'])) {
		deletepart($info);
	} else if (isset($_GET['uploadfile'])) {
		print_upload_form();
	} else if (isset($_POST['processuploadfile'])) {
		process_upload_file();
	} else if (isset($_GET['tools'])) {
		print_tools_page();
	} else if (isset($_REQUEST['approve'])) {
		print_approval_form($info);
	} else if (isset($_REQUEST['manage'])) {
		print_manage_form($info);
	} else {
		print_transact_form($info);
	}
	print("</div>"); // end of div main
}
print("</body></html>");
?>
