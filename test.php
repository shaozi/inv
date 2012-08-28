<html>
<head>
<script type='text/javascript' src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.1/jquery.min.js'>
</script>

<script type='text/javascript'>
$(document).ready(function(){
});
</script>

</head>
<body>
<form>
<table>
<tr>
<td>To:</td>
<td><input type='text' name='to'/></td>
</tr>
<tr>
<td>Send Name:</td>
<td><input type='text' name='name'/></td>
</tr>
<tr><td>From:</td><td><input type='text' name='from'/></td>
</tr>
</table>
<input type='submit'/>
</form>
</body>
</html>
<?php 

if (isset($_REQUEST['to']) && isset($_REQUEST['from']) &&
	isset($_REQUEST['name'])) {
	$from = $_REQUEST['from'];
	$to = $_REQUEST['to'];
	$sender = $_REQUEST['name'];
	$header = "Reply-To: $sender <$from@ixiacom.com>\r\n"; 
    $header .= "Return-Path: $sender <$from@ixiacom.com>\r\n"; 
    $header .= "From: $sender <$from@ixiacom.com>\r\n"; 
    $header .= "Organization: Ixia Communications\r\n"; 
    $header .= "Content-Type: text/plain\r\n"; 
 
    mail("$to@ixiacom.com", "Test Message", "This is my message.", $header); 
}
?>