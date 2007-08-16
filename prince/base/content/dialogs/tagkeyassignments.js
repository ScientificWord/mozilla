function clear(event)
{
  document.getElementById("newkeys").value = "";
  event.preventDefault();
}

function handleChar(event, object)
{
  var key=event.keyCode;
  if (key < event.DOM_VK_F1){
    clear(event);
    return;
  }
  if (key > event.DOM_VK_F24){
    clear(event);
    return;
  }
  var display="";
  var keycode = "F"+(event.keyCode - event.DOM_VK_F1 +1);
  if (event.shiftKey) display += "Shift+";
  if (event.ctrlKey) display += "Ctrl+";
  if (event.altKey) display += "Alt+";
  if (event.metaKey) display += "Meta+";
  display += keycode;
  // F1 and F10 are reserved, plain and shifted
  if ((key==event.DOM_VK_F1 || key==event.DOM_VK_F10) &&
    !(event.ctrlKey | event.altKey | event.metaKey))
  {
    AlertWithTitle("Reserved function key","The "+display+" key is reserved");
  }
  else
    document.getElementById("newkeys").value = display;
  event.preventDefault();
}

function radioswitched( obj )
{
  var taglisttextbox=document.getElementById("alltaglist");
  var keyscript=document.getElementById("keyscript");
  if (obj.value == 'alltaglist') {
    taglisttextbox.disabled = false;
    keyscript.disabled = true;
  }
  else
  {
    taglisttextbox.disabled = true;
    keyscript.disabled = false;
  }
}

function startUp()
{
}

function assignTag( obj )
{
  var editorElement = msiGetParentEditorElementForDialog(window);
  var editor = msiGetEditor(editorElement);
  var tag = obj.value;
  var tagmanager = editor.tagListManager;
  var data="";
  var class = tagmanager.getClassOfTag(tag,null);
  if (class == "texttag")
    data = "toggleTextTag('"+tag+"');";
  else if (class == "paratag")
    data = "insertParaTag('"+tag+"');";
  else if (class == "structtag")
    data = "insertSectionTag('"+tag+"');";
  else
    data = "insertTag('"+tag+"');";
  document.getElementById("keyscript").value = data;
}
  