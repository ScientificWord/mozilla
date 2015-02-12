
function startUp() {
	var editorElement = msiGetActiveEditorElement();
	var editor = msiGetEditor(editorElement);
	var hostid = editor.mAppUtils.hostid;
	document.getElementById('computerid').setAttribute('value', hostid);
}