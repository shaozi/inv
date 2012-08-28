<?php
include "lib/lib.inc";

session_start();
$dbh = db_init();
$result=authenticate($dbh);
session_commit();


print_html_header(MANAGEPAGE);


if($result!=PASS) {
	/* Not logged in */
	print_login_form();
	exit();
} else {

	//$info = get_info();

	// Now start main proc and print the real page

	/* logged in successfully */
	print_page_title();
	
}
?>

<div id='main'>

<h1 class='pagetitle'>User Group Management</h1>


<div id='message'></div>
<div>

<span class="buttonset">
<input type="button" id="creategroupbtn" value="Create Group"/>
<input type="button" id="deletegroupbtn" value="Delete Group"/>
</span>
<span class="buttonset">
<input type="button" id="addusertogroupbtn" value="Add User to Group"/>
<input type="button" id="deleteuserfromgroupbtn" value="Delete User from Group"/>
<input type="button" id="toggleuseradminbtn" value="Toggle Group Admin"/>
</span>
<input type="button" id="deleteuserbtn" value="Delete User"/>

</div>

<div id="creategroupdialog" title="Create New Group">
<p>
Group name: <input type='text' id='newgroupname'/>
</p>
</div>


<div id="allusers">
<h3 class='ui-widget-header ui-corner-top sec-title'>Users</h3>
<div class='ui-corner-bottom sec-content'>
<ul id='userlist' class='userlist'></ul>
</div>
</div>

<div id="allgroups">
<h3 class='ui-widget-header ui-corner-top sec-title'>Groups</h3>
<div class='ui-corner-bottom sec-content'>
<ul id='grouplist' class='grouplist'></ul>
</div>
</div>

<div id="groupmemberdiv">
<h3 class='ui-widget-header ui-corner-top sec-title'>Group Member</h3>
<div class='ui-corner-bottom sec-content'>
<ul id='groupmember'></ul>
</div>
</div>




</div>
</body>
</html>