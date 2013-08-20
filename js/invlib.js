/**
 * 
 */

function showmessage(selector, message, option){
	if (!message) return;
	
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

function wraperror() {
	$('.error').addClass("ui-state-error ui-corner-all");
	$('.error').prepend('<span class="ui-icon ui-icon-alert" style="float: left; margin-right: .3em;"></span>');
}
function wraphighlight() {
	$('.info').addClass("ui-state-highlight ui-corner-all");
	$('.info').prepend('<span class="ui-icon ui-icon-info" style="float: left; margin-right: .3em;"></span>');
}
function deconotice() {
	$('#notice').addClass("ui-state-highlight ui-corner-all");
	$('#notice').prepend('<span class="ui-icon ui-icon-info" style="float: left; margin-right: .3em;"></span>');
}
function decomenu() {
	//$('#menu').buttonset();
	//$('#menu a').button();
}
function decotable() {
	$('#partintable').tablesorter({widgets: ['zebra']});
	$('#partouttable').tablesorter({widgets: ['zebra']});
	
}
function decobutton() {
	$('input:submit, input:button, button').button();
	$('.buttonset').buttonset();
	$('input:text, textarea, input:password, select').addClass('ui-widget-content');
}
function decopanel() {
	$('.sec-content').addClass('ui-widget-content');
}
