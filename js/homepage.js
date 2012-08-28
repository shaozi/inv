/**
 * 
 */



var user_group;


function setuphomepage() { 
	$( "#customer" ).autocomplete({
		source: function( request, response ) {
			$.ajax({
				url: "searchcustomername.php",
				dataType: "json",
				data: {
					term: request.term
				},
				success: function( data ) {
					response( $.map( data, function( item ) {
						return {
							label: item.name + ", " + item.company,
							value: item.name,
							company: item.company
						};
					}));
				}
			});
		},
		minLength: 1,
		select: function( event, ui ) {
			$('#company').attr({'value':ui.item ? ui.item.company :''});
		}
	});

	$( "#serial" ).autocomplete({
		source: function( request, response ) {
			$.ajax({
				url: "searchpart.php",
				dataType: "json",
				data: {
					term: request.term
				},
				success: function( data ) {
					response( $.map( data, function( item ) {
						return {
							label: item.serial + ", " + item.model,
							value: item.serial,
							model: item.model
						};
					}));
				}
			});
		},
		minLength: 1,
		select: function( event, ui ) {
			$('#model').attr({'value':ui.item ? ui.item.model :''});
		}
	});
	
	
	$('.approve_request').click(function(){
		var serial = $(this).val();
		var this_button = $(this);
		$.ajax({
			url: "index_server.php",
			dataType: "json",
			data: {
				'action': "approve",
				'serial': serial
			},
			success: function( data ) {
				if (data.result==1) {
					this_button.parent().parent().remove();
					get_request_count();
					if ($('.approve_request').length==0) {
						$('#approvaltable').prev().html("No Requests");
						$('#approvaltable').remove();
					}
					showmessage("#message", 
							"Approved request", 
							"info");
				} else {
					
				}
			}
		});
	});
	
	$('.deny_request').click(function(){
		serial = $(this).val();
		var this_button = $(this);
		$.ajax({
			url: "index_server.php",
			dataType: "json",
			data: {
				'action': "deny",
				'serial': serial
			},
			success: function( data ) {
				if (data.result==1) {
					this_button.parent().parent().remove();
					get_request_count();
					if ($('.approve_request').length==0) {
						$('#approvaltable').prev().html("No Requests");
						$('#approvaltable').remove();
					}
					showmessage("#message", 
							"Denied request", 
							"info");
				} else {
					
				}
			}
		});
	});
	
	decomenu();
	
	decotable();
	decobutton();
	wraperror();
	wraphighlight();
}

function get_request_count() {
	$.ajax({
		url: "index_server.php",
		dataType: "json",
		data: {
			action: "get_request_count"
		},
		success: function( data ) {
			if (data.result==1 && data.count>0) {
				$('#notice').html("<a href='./index.php?approve=1'>"+data.count +" requests.</a>").show();
				deconotice();
			} else {
				$('#notice').empty().hide();
			}
		}
	});
}

$('document').ready(function(){
	/*
	 * for index.php
	 */
	get_request_count();
	setuphomepage();
	
});

