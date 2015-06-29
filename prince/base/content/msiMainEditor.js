#include productname.inc


// Copyright (c) 2006 MacKichan Software, Inc.  All Rights Reserved.
function msiShowHideToolbarSeparators(toolbar) {
  dump("===> msiShowHideToolbarSeparators\n");

  var childNodes = toolbar.childNodes;
  var separator = null;
  var hideSeparator = true;
  for (var i = 0; childNodes[i].localName != "spacer"; i++) {
    if (childNodes[i].localName == "toolbarseparator") {
      if (separator)
        separator.hidden = true;
      separator = childNodes[i];
    } else if (!childNodes[i].hidden) {
      if (separator)
        separator.hidden = hideSeparator;
      separator = null;
      hideSeparator = false;
    }
  }
}

function loadDocumentFromURI(pathString) {
  var newdocumentfile;
  var file;
  var url = msiFileURLFromAbsolutePath(pathString);
  file = msiFileFromFileURL(url);
  if (file && file.exists()){
    newdocumentfile = createWorkingDirectory(file);
    msiEditPage(msiFileURLFromFile(newdocumentfile), window, false, false);
  }
  else {
    // BBM: make this localizable
    AlertWithTitle("Recent file has been deleted", "The file "+file.leafName+" has been moved or deleted.");
    var popup = document.getElementById("menupopup_RecentFiles");
    if (popup){
      var items = popup.childNodes;
      var i;
      for (i = 0; i < items.length; i++)
      {
        if (pathString === items[i].getAttribute('value')){
          popup.removeChild(items[i]);
          break;
        }
      }
    }
    var historyCount = gPrefs.getIntPref("editor.history.url_maximum");
    var historyUrl;
    var thisUrl = msiFileURLStringFromFile( file );
    for (var j = 0; j < historyCount; j++)
    {
      historyUrl = GetUnicharPref("editor.history_url_"+j);
      if (historyUrl === thisUrl) {
        SetUnicharPref("editor.history_url_"+j, "");
        break;
      }
    }
  }
}


function msiShowHideToolbarButtons()
{
  dump("===> msiShowHideToolbarButtons\n");

  var array = GetPrefs().getChildList(kEditorToolbarPrefs, {});
  for (var i in array) {
    var prefName = array[i];
    var id = prefName.substr(kEditorToolbarPrefs.length) + "Button";
    var button = document.getElementById(id);
    if (button)
      button.hidden = !gPrefs.getBoolPref(prefName);
  }
  msiShowHideToolbarSeparators(document.getElementById("EditToolbar"));
  msiShowHideToolbarSeparators(document.getElementById("FormatToolbar"));
}


function msiMainWindowMouseDownListener(event)
{
  var target = event.explicitOriginalTarget;
  if (!target)
    target = event.originalTarget;
//  if (!target)
//    return;

  var changeActiveEditor = true;
  var nodeStr = "";
  if (target)
  {
    switch(target.nodeName)
    {
      case "menu":
      case "menulist":
      case "menuitem":
      case "menupopup":
      case "toolbar":
      case "toolbarbutton":
      case "toolbarpalette":
      case "toolbaritem":
        changeActiveEditor = false;
      break;
      case "input":
      case "html:input":
      case "textbox":
        if (msiFindParentOfType(target, "toolbar", "window"))
          changeActiveEditor = false;
      break;
      default:
      break;
    }
    nodeStr = target.nodeName;
  }
  var logStr = "In msiMainWindowMouseDownListener, target.nodeType is [" + nodeStr + "]\n";
  msiKludgeLogString(logStr);
//  window.bIgnoreNextFocus = !changeActiveEditor;
  if (!changeActiveEditor)
  {
    msiResetActiveEditorElement(this.document.defaultView);
  }
}

//TO DO - find out whether this ever happens (appears not to be called from anywhere)
//function AfterHighlightColorChange()
//{
//  if (!IsHTMLEditor())
//    return;
//
//  var button = document.getElementById("cmd_highlight");
//  if (button) {
//    var mixedObj = {};
//    try {
//      var state = GetCurrentEditor().getHighlightColorState(mixedObj);
//      button.setAttribute("state", state);
//      onHighlightColorChange();
//    } catch (e) {}
//  }
//}

function initSidebars()
{
  var sidebaritems = document.getElementById("sidebaritems");
  var sidebar1 = document.getElementById("sidebar1");
  var sidebar2 = document.getElementById("sidebar2");
  var splitter1 = document.getElementById("splitter1");
  var splitter2 = document.getElementById("splitter2");
  sidebaritems.init();
  sidebar1.init(sidebaritems, sidebar2, splitter1);
  sidebar2.init(sidebaritems, sidebar1, splitter2);
}

function msiEditorOnLoad()
{

  dump("\n=====================EditorOnLoad\n");
  dump(window.arguments[0]+"\n");
  dump(window.arguments.length+"\n");
  // See if argument was passed.
  if ( window.arguments && window.arguments[0] )
  {
    // The following is a hack. When Prince starts up, there seems to be a single argument attached which is not a string,
    // but a C++ object wrapped by JavaScript. The following line forces conversion to a string and then searches it for
    // 'xpconnect wrapped'. Ugly, but it works. It would be better to find the offending first call.
    if ((window.arguments[0]+"@@@").search("xpconnect wrapped")<0)
    // Opened via window.openDialog with URL as argument.
    // Put argument where EditorStartup expects it.
    {
      document.getElementById( "args" ).setAttribute( "value", window.arguments[0] );
    }
  }
  if ( window.arguments && window.arguments[3] )
  {
      document.getElementById( "args" ).setAttribute( "isShell", window.arguments[3] );
  }

  // get default character set if provided
  if ("arguments" in window && window.arguments.length > 1)
  {
    var arrayArgComponents;
    if (window.arguments[1] && window.arguments[1].indexOf("charset=") != -1)
    {
      arrayArgComponents = window.arguments[1].split("=");
      if (arrayArgComponents)
      {
        // Put argument where EditorStartup expects it.
        document.getElementById( "args" ).setAttribute("charset", arrayArgComponents[1]);
      }
    }
    if ((window.arguments.length > 2) && window.arguments[2] && window.arguments[2].indexOf("initialMarker=") != -1)
    {
      arrayArgComponents = window.arguments[2].split("=");
      if (arrayArgComponents)
      {
        // Put argument where EditorStartup expects it.
        document.getElementById( "args" ).setAttribute("initialMarker", arrayArgComponents[1]);
      }
    }
  }

  window.tryToClose = msiEditorCanClose;
  window.addEventListener("mousedown", msiMainWindowMouseDownListener, true);

  // Continue with normal startup.
  var editorElement = document.getElementById("content-frame");
  editorElement.fastCursorInit = function() {initFastCursorBar();}
  EditorStartupForEditorElement(editorElement);
  msiRemoveInapplicableUIElements(editorElement);
  initSidebars();

  // Initialize our source text <editor>
  try {
//    editorElement.mSourceContentWindow = document.getElementById("content-source");
//    editorElement.mSourceContentWindow.makeEditable("text", false);
//    editorElement.mSourceTextEditor = editorElement.mSourceContentWindow.getEditor(editorElement.mSourceContentWindow.contentWindow);
//    editorElement.mSourceTextEditor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
//    editorElement.mSourceTextEditor.enableUndo(false);
//    editorElement.mSourceTextEditor.rootElement.style.fontFamily = "-moz-fixed";
//    editorElement.mSourceTextEditor.rootElement.style.whiteSpace = "pre";
//    editorElement.mSourceTextEditor.rootElement.style.margin = 0;
//    //ljh I don't understand why a new instance of the controller is being created?????
//    var controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"]
//                               .createInstance(Components.interfaces.nsIControllerContext);
//    controller.init(null);
//    controller.setCommandContext(editorElement.mSourceContentWindow);
//    editorElement.mSourceContentWindow.contentWindow.controllers.insertControllerAt(0, controller);
//    var commandTable = controller.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
//                                 .getInterface(Components.interfaces.nsIControllerCommandTable);
//    commandTable.registerCommand("cmd_find",        msiFindCommand);
//    commandTable.registerCommand("cmd_findNext",    msiFindAgainCommand);
//    commandTable.registerCommand("cmd_findPrev",    msiFindAgainCommand);
//
    msiSetupMSIMathMenuCommands(editorElement);
#ifndef PROD_SW
    if (msiSetupMSIComputeMenuCommands) msiSetupMSIComputeMenuCommands(editorElement);
#endif
#ifndef PROD_SNB
    msiSetupMSITypesetMenuCommands(editorElement);
    msiSetupMSITypesetInsertMenuCommands(editorElement);
#endif
  } catch (e)
  {
    dump("makeEditable failed: "+e+"\n");
  }
}


// --------------------------- File menu ---------------------------


//// used by openLocation. see openLocation.js for additional notes.
//function delayedOpenWindow(chrome, flags, url)
//{
//  dump("setting timeout\n");
//  setTimeout("window.openDialog('"+chrome+"','_blank','"+flags+"','"+url+"')", 10);
//}
//
//function EditorNewPlaintext()
//{
//  window.openDialog( "chrome://editor/content/TextEditorAppShell.xul",
//                     "_blank",
//                     "chrome,dialog=no,all",
//                     "about:blank");
//}

// Check for changes to document and allow saving before closing
// This is hooked up to the OS's window close widget (e.g., "X" for Windows)
function msiEditorCanClose(editorElement)
{
  // Returns FALSE only if user cancels save action
  if (!editorElement)
    editorElement = msiGetCurrentEditorElementForWindow(window);

  // "true" means allow "Don't Save" button
  var canClose = msiCheckAndSaveDocument(editorElement, "cmd_close", true);

  // This is our only hook into closing via the "X" in the caption
  //   or "Quit" (or other paths?)
  //   so we must shift association to another
  //   editor or close any non-modal windows now
  if (canClose && "InsertCharWindow" in window && window.InsertCharWindow)
    SwitchInsertCharToAnotherEditorOrClose();
  if (canClose) ShutdownAnEditor(editorElement);
  return canClose;
}

// --------------------------- View menu ---------------------------

// --------------------------- Text style ---------------------------

//Window title, file menu updating.

function UpdateWindowTitle()
{
  msiUpdateWindowTitle();
}

function msiUpdateWindowTitle()
{
  try
  {
    var editorElement = msiGetTopLevelEditorElement();
    var fileName;
    if (editorElement.isShellFile) fileName = "not saved";
    else //if (!fileName)
    {
//      filename = editorElement.fileLeafName;
			var htmlurlstring = msiGetEditorURL(editorElement);
		  var sciurlstring = msiFindOriginalDocname(htmlurlstring);
		  var fileURL = msiURIFromString(sciurlstring);
			var file = msiFileFromFileURL(fileURL);
			var leaf = file.leafName;
    }
//    else filename = fileName;
//    SaveRecentFilesPrefs();
    // Set window title with " - Scientific WorkPlace/Word/Notebook" appended
    var xulWin = document.getElementById('prince');
    if (!leaf) leaf="untitled";
    document.title = leaf + xulWin.getAttribute("titlemodifierseparator") +
                   xulWin.getAttribute("title");
  } catch (e)
  {
    dump(e+"\n");
  }
}

function BuildRecentFilesMenu()
{
  var editorElement = msiGetTopLevelEditorElement();
  var editor = msiGetEditor(editorElement);
//  var editor = GetCurrentEditor();
  if (!editor || !gPrefs)
    return;

  var popup = document.getElementById("menupopup_RecentFiles");
  if (!popup || !editor.document)
    return;

  // Delete existing menu
  while (popup.firstChild)
    popup.removeChild(popup.firstChild);

  // Current page is the "0" item in the list we save in prefs,
  //  but we don't include it in the menu.
  var curUrl = StripPassword(msiGetEditorURL(editorElement));
  var historyCount = 10;
  try {
    historyCount = gPrefs.getIntPref("editor.history.url_maximum");
  } catch(e) {}
  var menuIndex = 1;

  for (var i = 0; i < historyCount; i++)
  {
    var url = GetUnicharPref("editor.history_url_"+i);

    // Skip over current url
    if (url && url != curUrl && url.length > 0)
    {
      // Build the menu
      var title = GetUnicharPref("editor.history_title_"+i);
			var path = msiPathFromFileURL(msiURIFromString(url));
      var fname = path.replace(/^.*\//,"");
      if (fname.indexOf('.sci') == fname.length-4) {
        AppendRecentMenuitem(popup, fname, path, menuIndex);
        menuIndex++;
      }
    }
  }
}

function SaveRecentFilesPrefs()
{
  // Can't do anything if no prefs
  if (!gPrefs) return;

  var editorElement = msiGetTopLevelEditorElement();
  var curUrl = unescape(StripPassword(msiGetEditorURL(editorElement)));
  var historyCount = 10;
  var origName;
  try {
    historyCount = gPrefs.getIntPref("editor.history.url_maximum");
  } catch(e) {}

  // var titleArray = [];
  var urlArray = [];

  if (historyCount && !IsUrlAboutBlank(curUrl) &&  GetScheme(curUrl) != "data")
  {
    origName = msiFindOriginalDocname(curUrl);
    if (origName.indexOf(".sci") == origName.length-4){
      // titleArray.push(unescape(msiGetDocumentTitle(editorElement)));
      urlArray.push(origName);
    }
  }

  for (var i = 0; i < historyCount && urlArray.length < historyCount; i++)
  {
    var url = unescape(GetUnicharPref("editor.history_url_"+i));

    // Continue if URL pref is missing because
    // a URL not found during loading may have been removed

    // Skip over current and "data" URLs and ""
    if (url && url.length > 0 && url != curUrl && GetScheme(url) != "data")
    {
      // var title = unescape(GetUnicharPref("editor.history_title_"+i));
      var duplicate = false;
      for (var j = 0; j <urlArray.length; j++)
      {
        // if (title == titleArray[j] && url == urlArray[j])
        if (url == urlArray[j])
        {
          duplicate = true;
          break;
        }
      }
      if (!duplicate) {
 //       titleArray.push(title);
        urlArray.push(url);
      }
    }
  }

  // Resave the list back to prefs in the new order
  for (i = 0; i < urlArray.length; i++)
  {
//    SetUnicharPref("editor.history_title_"+i, titleArray[i]);
    SetUnicharPref("editor.history_url_"+i, urlArray[i]);
  }
}

function AppendRecentMenuitem(menupopup, title, url, menuIndex)
{
  if (menupopup)
  {
    var menuItem = document.createElementNS(XUL_NS, "menuitem");
    if (menuItem)
    {
      var accessKey;
      if (menuIndex <= 9)
        accessKey = String(menuIndex);
      else if (menuIndex == 10)
        accessKey = "0";
      else
        accessKey = " ";

      var itemString = accessKey+" ";

      // Show "title [url]" or just the URL
//      if (title)
//      {
//       itemString += title;
//       itemString += " [";
//      }
      itemString += title;
//      if (title)
//        itemString += "]";

      menuItem.setAttribute("label", itemString);
      menuItem.setAttribute("crop", "center");
      menuItem.setAttribute("value", url);
      if (accessKey != " ")
        menuItem.setAttribute("accesskey", accessKey);
      menupopup.appendChild(menuItem);
    }
  }
}

//TO DO: Qant to disable some (or all?) File Menu items for subsidiary editors?
function msiEditorInitFileMenu()
{
  // Disable "Save" menuitem when editing remote url. User should use "Save As"
  var editorElement = msiGetTopLevelEditorElement();
  var docUrl = msiGetEditorURL(editorElement);
  var scheme = GetScheme(docUrl);
  if (scheme && scheme != "file")
    SetElementEnabledById("saveMenuitem", false);

  // Enable recent pages submenu if there are any history entries in prefs
  var historyUrl = "";

  var historyCount = 10;
  try { historyCount = gPrefs.getIntPref("editor.history.url_maximum"); } catch(e) {}
  if (historyCount)
  {
    historyUrl = GetUnicharPref("editor.history_url_0");

    // See if there's more if current file is only entry in history list
    if (historyUrl && historyUrl == docUrl)
      historyUrl = GetUnicharPref("editor.history_url_1");
  }
  SetElementEnabledById("menu_RecentFiles", historyUrl != "");
  msiGoUpdateComposerMenuItems(document.getElementById("composerMenuItems"));
}

// --------------------------- Logging stuff ---------------------------


//// --------------------------- Status calls ---------------------------
////-----------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------
//This function is needed only for the top level editor?
function msiRemoveInapplicableUIElements(editorElement)
{
  if (!editorElement)
    editorElement = msiGetTopLevelEditorElement();  //Use the main window's editor unless someone passes one in.
  // For items that are in their own menu block, remove associated separator
  // (we can't use "hidden" since class="hide-in-IM" CSS rule interferes)

   // if no find, remove find ui
  if (!msiIsFindInstalled())
  {
    msiHideItem("menu_find");
    msiHideItem("menu_findnext");
    msiHideItem("menu_replace");
    msiHideItem("menu_find");
    msiRemoveItem("sep_find");
  }

   // if no spell checker, remove spell checker ui
  if (!msiIsSpellCheckerInstalled())
  {
    SetElementEnabled(document.getElementById("spellingButton"), false);
    msiHideItem("menu_checkspelling");
    msiRemoveItem("sep_checkspelling");
  }
  else
  {
    SetElementEnabled(document.getElementById("menu_checkspelling"), true);
    SetElementEnabled(document.getElementById("spellingButton"), true);
    SetElementEnabled(document.getElementById("checkspellingkb"), true);
  }

  // Remove menu items (from overlay shared with HTML editor) in non-HTML.
  if (!msiIsHTMLEditor(editorElement))
  {
    msiHideItem("insertAnchor");
    msiHideItem("insertImage");
    msiHideItem("insertHline");
    msiHideItem("insertTable");
    msiHideItem("insertHTML");
    msiHideItem("insertFormMenu");
    msiHideItem("fileExportToText");
    msiHideItem("viewFormatToolbar");
    msiHideItem("viewEditModeToolbar");
  }
}

//NOTE: Following don't need to be overwritten. However, we do to avoid having to load editor.js as well as this.
function msiHideItem(id)
{
  var item = document.getElementById(id);
  if (item)
    item.hidden = true;
}

function msiShowItem(id)
{
  var item = document.getElementById(id);
  if (item)
    item.hidden = false;
}

function msiRemoveItem(id)
{
  var item = document.getElementById(id);
  if (item)
    item.parentNode.removeChild(item);
}


function msiEditorWindowOnFocus(event)
{
  var commandDispatcher = window.document.commandDispatcher;
  var currElement = commandDispatcher.focusedElement;
  var changeActiveEditor = true;
  var targetElement = event.explicitOriginalTarget;
  if (targetElement == null)
    targetElement = event.originalTarget;
  var bFound = false;
  var targsToCheck = new Array(currElement);
  if ((targetElement != null) && (targetElement != currElement))
    targsToCheck.push(targetElement);
  for (var ix = 0; !bFound && ix < targsToCheck.length; ++ix)
  {
    if ( (targsToCheck[ix] == null) || !("nodeName" in targsToCheck[ix]) )
      continue;
    switch(targsToCheck[ix].nodeName)
    {
      case "#document":
      case "menu":
      case "menulist":
      case "menuitem":
      case "menupopup":
      case "toolbar":
      case "toolbarbutton":
      case "toolbarpalette":
      case "toolbaritem":
      case "listbox":
      case "listitem":
      case "listcell":
        changeActiveEditor = false;
        bFound = true;
      break;
      case "input":
      case "html:input":
      case "textbox":
        bFound = true;
        if (msiFindParentOfType(currElement, "toolbar", "window"))
          changeActiveEditor = false;
      break;
      case "":
        changeActiveEditor = false;
      break;
      default:
        bFound = true;
      break;
    }
  }
//Logging stuff only
  if (!bFound)
    changeActiveEditor = false;

  var logString = msiEditorStateLogString(window);
  logString += " Focus element reported as [";
  if ((currElement != null) && ("nodeName" in currElement))
    logString += currElement.nodeName;
  logString += "] in msiEditorOnFocus; ";
  if (targsToCheck.length > 1)
    logString += "event target is [";
  if ((targsToCheck[1] != null) && ("nodeName" in targsToCheck[1]))
    logString += targsToCheck[1].nodeName;
  logString += "]; changeActiveEditor is [";
  if (changeActiveEditor)
    logString += "true";
  else
    logString += "false";
  msiKludgeLogString(logString + "].\n");
//End logging stuff only

  if (changeActiveEditor)
  {
//    var editorElement = GetCurrentEditorElement();
    var editorElement = msiGetCurrentEditorElementForWindow(window);
    msiSetActiveEditor(editorElement, true);
  }
  else
  {
    msiResetActiveEditorElement(this.document.defaultView);
  }

//  //Finally call "EditorOnFocus" on our containing window - for the time being anyway.
//  //This resets all the "InsertCharWindow" stuff. TO DO: redo it.
//  EditorOnFocus();

//  // Current window already has the InsertCharWindow
//  if ("InsertCharWindow" in window && window.InsertCharWindow) return;
//
//  // Find window with an InsertCharsWindow and switch association to this one
//  var windowWithDialog = FindEditorWithInsertCharDialog();
//  if (windowWithDialog)
//  {
//    // Switch the dialog to current window
//    // this sets focus to dialog, so bring focus back to editor window
//    if (SwitchInsertCharToThisWindow(windowWithDialog))
//      top.document.commandDispatcher.focusedWindow.focus();
//  }
}


function UpdateTOC()
{
  window.openDialog("chrome://editor/content/EdInsertTOC.xul",
                    "inserttoc", "chrome,close,modal,resizable,titlebar");
  window._content.focus();
}

function InitTOCMenu()
{
  var elt = msiGetCurrentEditor().document.getElementById("mozToc");
  var createMenuitem = document.getElementById("insertTOCMenuitem");
  var updateMenuitem = document.getElementById("updateTOCMenuitem");
  var removeMenuitem = document.getElementById("removeTOCMenuitem");
  if (removeMenuitem && createMenuitem && updateMenuitem) {
    if (elt) {
      createMenuitem.setAttribute("disabled", "true");
      updateMenuitem.removeAttribute("disabled");
      removeMenuitem.removeAttribute("disabled");
    }
    else {
      createMenuitem.removeAttribute("disabled");
      removeMenuitem.setAttribute("disabled", "true");
      updateMenuitem.setAttribute("disabled", "true");
    }
  }
}

function RemoveTOC()
{
  var theDocument = msiGetCurrentEditor().document;
  var elt = theDocument.getElementById("mozToc");
  if (elt) {
    elt.parentNode.removeChild(elt);
  }

  function acceptNode(node)
  {
    if (node.nodeName.toLowerCase() == "a" &&
        node.hasAttribute("name") &&
        node.getAttribute("name").substr(0, 8) == "mozTocId") {
      return NodeFilter.FILTER_ACCEPT;
    }
    return NodeFilter.FILTER_SKIP;
  }

  var treeWalker = theDocument.createTreeWalker(theDocument.documentElement,
                                                NodeFilter.SHOW_ELEMENT,
                                                acceptNode,
                                                true);
  if (treeWalker) {
    var anchorNode = treeWalker.nextNode();
    while (anchorNode) {
      var tmp = treeWalker.nextNode();
      anchorNode.parentNode.removeChild(anchorNode);
      anchorNode = tmp;
    }
  }
}


