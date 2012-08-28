/**
 * 
 */

function showmessage(selector, message, option){
	var msgdiv = $("<div/>");
	if (option=='error') {
		msgdiv.addClass("ui-state-error ui-corner-all");
		msgdiv.html(message);
		msgdiv.prepend('<span class="ui-icon ui-icon-alert" style="float: left; margin-right: .3em;"></span>');
	} else {
		msgdiv.addClass("ui-state-highlight ui-corner-all");
		msgdiv.html(message);
		msgdiv.prepend('<span class="ui-icon ui-icon-info" style="float: left; margin-right: .3em;"></span>');
	}
	msgdiv.appendTo($(selector)).show();
	setTimeout(function(){msgdiv.remove();}, 5000);
}

var selected_group="";
var selected_user="";
var selected_user_in_group="";
var user_realname=new Object;


function refresh() {
	if (selected_group!="") {
		$('#allgroups [name="'+selected_group+'"]').click();
	}
	if (selected_user!="") {
		$('#allusers [name="'+selected_user+'"]').click();
	}
	if (selected_user_in_group!="") {
		$('#groupmemberdiv [name="'+selected_user_in_group+'"]').click();
	}
}

function getusergroupmap() {
	$.ajax({
		'url':'manage_server.php',
		'data':{
			'action': 'listusergroup'
		},
		'success':function(data) {
			if (data.result==0) {
				showmessage('#usergroupdiv', "Cannot get user group map. "+
						"Error message: $data.message", 'error');
			} else {
				$('#userlist').empty();
				$('#grouplist').empty();
				$.each(data.users, function(i, v) {
					if (v.active==0) {
						return;
					}
					var user = $("<li class='user' name='"+i+"'/>");
					user.html(v.realname);
					
					
						user_realname[i]=v.realname;
					
						user.click(function(){
							$(".user").removeClass('ui-state-highlight');
							user.addClass('ui-state-highlight');
							selected_user = i;
							$('.group').each(function(){
								$(this).css({'font-weight':'normal'});
								for (var gindex=0; gindex<v.groups.length; gindex++) {
									if ($(this).attr('name') == v.groups[gindex]) {
										$(this).css({'font-weight':'bold'});
										break;
									}
								}
							});
							
						});
					
					$('#userlist').append(user);
				});
				$.each(data.groups, function(i, v){
					var group = $("<li class='group' name='"+i+"'/>");
					
					group.html(i+" ("+ v.length + (v.length>1?" members)":" member)"));
					group.click(function(){
						$(".group").removeClass('ui-state-highlight');
						group.addClass('ui-state-highlight');
						$("#groupmember").empty();
						for (index=0; index<v.length; index++) {
							var member = $("<li class='user' name='"+v[index].user+"'/>");
							if (v[index].isadmin==1) {
								member.html("<b>"+user_realname[v[index].user]+"</b>");
							} else {
								member.html(user_realname[v[index].user]);
							}
							member.click(function(){
								$(".user").removeClass('ui-state-highlight');
								$(this).addClass('ui-state-highlight');
								selected_user_in_group= $(this).attr('name');
							});
							$("#groupmember").append(member);
						}
						selected_group=i;
					});
					$('#grouplist').append(group);
				});
				$('#groupmember').empty();
				refresh();
			}
		},
		'dataType': 'json'
	});		
}

function initCreateGroupDialog() {
	$('#creategroupdialog').dialog({
		modal: true,
		buttons: {
			"Create Group": function() {
				var groupname =
					$('#newgroupname').attr('value');
				$.ajax({
					'url':'manage_server.php',
					'data': {
						'action': 'creategroup', 
						'groupname': groupname
					},
					'success':function(data) {
						
						if (data.result==1) {
							showmessage("#message", 
									"Successfully created group '"+
									groupname+"'.", 
									"info");
							getusergroupmap();
							selected_group=groupname;
							refresh();
						} else {
							showmessage("#message", 
									"Failed to create group '"+
									groupname+"'. Message from server: " +
									data.message, "error");
						}
					},
					'dataType': 'json'
				});
				$( this ).dialog( "close" );
			},
			Cancel: function() {
				$( this ).dialog( "close" );
			}
		},
		autoOpen: false
	});
}

function deleteUser(username) {
	var allusers = $("<div title='Choose a new user'/>");
	var explain = $('<p>');
	explain.html('Please choose a user to take over all items.');
	explain.appendTo(allusers);
	var selusers = $("<select/>");
	selusers.appendTo(allusers);
	$.each(user_realname, function(i,v){
		if (username!=i) {
			var opt = $("<option value='"+i+"'/>");
			opt.html(v);
			opt.appendTo(selusers);
		}
	});
	allusers.dialog({
		modal: true,
		buttons: {
			"Delete User": function() {
				var transferusername =
					$('option:selected', selusers).attr('value');
				$.ajax({
					'url':'manage_server.php',
					'data': {
						'action': 'deleteuser', 
						'username': username,
						'transferusername': transferusername
					},
					'success':function(data) {
						
						if (data.result==1) {
							showmessage("#message", 
									"Successfully deleted user", 
									"info");
							selected_user = "";
							delete user_realname[username];
							getusergroupmap();
						} else {
							showmessage("#message", 
									data.message, "error");
						}	
					},
					'dataType': 'json'
				});		
				
				$( this ).dialog( "close" );
				$( this ).remove();
				
			},
			Cancel: function() {
				$( this ).dialog( "close" );
				$( this ).remove();
				return;
			}
		}
	});
	
	
}

function deleteGroup(groupname) {
	$.ajax({
		'url':'manage_server.php',
		'data': {
			'action': 'deletegroup', 
			'groupname': groupname
		},
		'success':function(data) {
			
			if (data.result==1) {
				showmessage("#message", 
						"Successfully deleted group '"+groupname+"'.", 
						"info");
				getusergroupmap();
			} else {
				showmessage("#message", 
						"Failed to delete group '"+
						groupname+"'. Message from server: " +
						data.message, "error");
			}
			selected_group = "";
		},
		'dataType': 'json'
	});		
}

function addUserToGroup(username, groupname) {
	$.ajax({
		'url':'manage_server.php',
		'data': {
			'action': 'addusertogroup', 
			'groupname': groupname,
			'username': username
		},
		'success':function(data) {
			
			if (data.result==1) {
				showmessage("#message", 
						"Successfully added "+username+" to group '"+groupname+"'.", 
						"info");
				getusergroupmap();
			} else {
				showmessage("#message", 
						"Failed to add user "+username+" to group '"+
						groupname+"'. Message from server: " +
						data.message, "error");
			}
		},
		'dataType': 'json'
	});		
}

function deleteUserFromGroup(username, groupname) {
	$.ajax({
		'url':'manage_server.php',
		'data': {
			'action': 'deleteuserfromgroup', 
			'groupname': groupname,
			'username': username
		},
		'success':function(data) {
			
			if (data.result==1) {
				showmessage("#message", 
						"Successfully deleted "+username+" from group '"+
						groupname+"'.", 
						"info");
				getusergroupmap();
			} else {
				showmessage("#message", 
						"Failed to delete user "+username+" from group '"+
						groupname+"'. Message from server: " +
						data.message, "error");
			}
		},
		'dataType': 'json'
	});		
}

function toggleUserGroupAdmin(username, groupname) {
	$.ajax({
		'url':'manage_server.php',
		'data': {
			'action': 'toggleusergroupadmin', 
			'groupname': groupname,
			'username': username
		},
		'success':function(data) {
			
			if (data.result==1) {
				showmessage("#message", 
						data.message, 
						"info");
				getusergroupmap();
			} else {
				showmessage("#message", 
						data.message, "error");
			}
		},
		'dataType': 'json'
	});		
}
function setupmanagepage() {
	initCreateGroupDialog();
	
	$('#creategroupbtn').click(function(){
		$('#creategroupdialog').dialog('open');
	});
	
	$('#deleteuserbtn').click(function(){
		if (selected_user=="") {
			return;
		}
		deleteUser(selected_user);
	});
	$('#deletegroupbtn').click(function(){
		if (selected_group=="") {
			return;
		}
		deleteGroup(selected_group);
	});
	$('#addusertogroupbtn').click(function(){
		if (selected_user=="" || selected_group=="") {
			return;
		}
		addUserToGroup(selected_user, selected_group);
	});
	$('#deleteuserfromgroupbtn').click(function(){
		if (selected_user_in_group=="" || selected_group=="") {
			return;
		}
		deleteUserFromGroup(selected_user_in_group, selected_group);
	});
	
	$('#toggleuseradminbtn').click(function(){
		if (selected_user_in_group=="" || selected_group=="") {
			return;
		}
		toggleUserGroupAdmin(selected_user_in_group, selected_group);
	});
	
	getusergroupmap();
}


$('document').ready(function(){
	/*
	 * for manage.php
	 */
	setupmanagepage();
	decobutton();
	decomenu();
	decopanel();
});

