<?php 
include "lib/lib.inc";
session_start();
db_init(null);
session_commit();

print_html_header(HELP);
print_page_title();

?>
<div id="main">
<h1>User Guide</h1>
<h2>Users, Groups, Owners, and Admins</h2>

<p><ul>
<li>A user choose a group to join when he or she register to the database</li>
<li>All users in the same group can view each other's items.</li>
<li>Only the owner of the item or the admin of the group can check in or out that item</li>
<li>When a non-privilege user try to check in or check out an item,
a request email will be automatically sent to the owner and all admins in the group. 
Any of them can approve or deny the request through the web page</li>
<li>A user can belongs to multiple groups</li>
<li>A group can have zero, one or multiple admins</li>
<li>An admin in one group can be a regular user in another group</li>
<li>A superuser can manage users, admins and groups</li>
</ul></p>

</div>
</body>
</html>