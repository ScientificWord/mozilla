
function accept ()
{
	return true;
}


function onLoad()
{
	var message = window.arguments[0];
	//message = message.replace(/\n/g, '<br/>');
	document.getElementById('errors').value = message;
}
