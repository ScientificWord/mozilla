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

  // get default character set if provided
  if ("arguments" in window && window.arguments.length > 1 && window.arguments[1])
  {
    if (window.arguments[1].indexOf("charset=") != -1)
    {
      var arrayArgComponents = window.arguments[1].split("=");
      if (arrayArgComponents)
      {
        // Put argument where EditorStartup expects it.
        document.getElementById( "args" ).setAttribute("charset", arrayArgComponents[1]);
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
    editorElement.mSourceContentWindow = document.getElementById("content-source");
    editorElement.mSourceContentWindow.makeEditable("text", false);
    editorElement.mSourceTextEditor = editorElement.mSourceContentWindow.getEditor(editorElement.mSourceContentWindow.contentWindow);
    editorElement.mSourceTextEditor.QueryInterface(Components.interfaces.nsIPlaintextEditor);
    editorElement.mSourceTextEditor.enableUndo(false);
    editorElement.mSourceTextEditor.rootElement.style.fontFamily = "-moz-fixed";
    editorElement.mSourceTextEditor.rootElement.style.whiteSpace = "pre";
    editorElement.mSourceTextEditor.rootElement.style.margin = 0;
    //ljh I don't understand why a new instance of the controller is being created?????
    var controller = Components.classes["@mozilla.org/embedcomp/base-command-controller;1"]
                               .createInstance(Components.interfaces.nsIControllerContext);
    controller.init(null);
    controller.setCommandContext(editorElement.mSourceContentWindow);
    editorElement.mSourceContentWindow.contentWindow.controllers.insertControllerAt(0, controller);
    var commandTable = controller.QueryInterface(Components.interfaces.nsIInterfaceRequestor)
                                 .getInterface(Components.interfaces.nsIControllerCommandTable);
                                 dump("\n13");
    commandTable.registerCommand("cmd_find",        msiFindCommand);
    commandTable.registerCommand("cmd_findNext",    msiFindAgainCommand);
    commandTable.registerCommand("cmd_findPrev",    msiFindAgainCommand);
    
    msiSetupMSIMathMenuCommands(editorElement);
    msiSetupMSIComputeMenuCommands(editorElement);
    msiSetupMSITypesetMenuCommands(editorElement);
    msiSetupMSITypesetInsertMenuCommands(editorElement);
  } catch (e) { dump("makeEditable failed: "+e+"\n"); }
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
  try {
    var editorElement = msiGetTopLevelEditorElement();
    var windowTitle;
     windowTitle = msiGetDocumentTitle(editorElement);
     if (!windowTitle)
       windowTitle = GetString("untitled");

    // Append just the 'leaf' filename to the Doc. Title for the window caption
    var docUrl = msiGetEditorURL(editorElement); //docUrl is a string
    if (docUrl) // && !IsUrlAboutBlank(docUrl))
    {
      docUrl = msiFindOriginalDocname(docUrl);
      var path = unescape(docUrl);
      var regEx = /\/main.xhtml$/i;  // BBM: localize this
      if (regEx.test(path))
      {
        var parentDirRegEx = /(.*\/)([A-Za-z0-9_\b\-]*)_work\/main.xhtml$/i; //BBM: localize this
        var arr = parentDirRegEx.exec(path);
        if ((arr.length > 2) && arr[2].length > 0)
          docUrl = arr[1]+arr[2]+".sci";
      }

      var scheme = GetScheme(docUrl);
      var filename  = GetFilename(docUrl);
      if (!filename || filename.length == 0)
        filename = GetFilename(docUrl);

      // Save changed title in the recent pages data in prefs
      SaveRecentFilesPrefs();
    }
    // Set window title with " - Scientific WorkPlace/Word/Notebook" appended
    var xulWin = document.documentElement;
    document.title = filename + " ["+windowTitle+"] " + xulWin.getAttribute("titlemenuseparator") + 
                   xulWin.getAttribute("titlemodifier");
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
    if (url && url != curUrl)
    {
      // Build the menu
      var title = GetUnicharPref("editor.history_title_"+i);
      AppendRecentMenuitem(popup, title, url, menuIndex);
      menuIndex++;
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
  try {
    historyCount = gPrefs.getIntPref("editor.history.url_maximum"); 
  } catch(e) {}

  var titleArray = [];
  var urlArray = [];

  if (historyCount && !IsUrlAboutBlank(curUrl) &&  GetScheme(curUrl) != "data")
  {
    titleArray.push(unescape(msiGetDocumentTitle(editorElement)));
    urlArray.push(msiFindOriginalDocname(curUrl));
  }

  for (var i = 0; i < historyCount && urlArray.length < historyCount; i++)
  {
    var url = GetUnicharPref("editor.history_url_"+i);

    // Continue if URL pref is missing because 
    //  a URL not found during loading may have been removed

    // Skip over current an "data" URLs
    if (url && url != curUrl && GetScheme(url) != "data")
    {
      var title = GetUnicharPref("editor.history_title_"+i);
      titleArray.push(unescape(title));
      urlArray.push(unescape(url));
    }
  }

  // Resave the list back to prefs in the new order
  for (i = 0; i < urlArray.length; i++)
  {
    SetUnicharPref("editor.history_title_"+i, titleArray[i]);
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
      if (title)
      {
       itemString += title;
       itemString += " [";
      }
      itemString += url;
      if (title)
        itemString += "]";

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


function msiEditorWindowOnFocus()
{
  var commandDispatcher = window.document.commandDispatcher;
  var currElement = commandDispatcher.focusedElement;
  var changeActiveEditor = true;
  if (currElement)
  {
    switch(currElement.nodeName)
    {
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
      break;
      case "input":
      case "html:input":
      case "textbox":
        if (msiFindParentOfType(currElement, "toolbar", "window"))
          changeActiveEditor = false;
      break;
    }
//Logging stuff only
    var logString = "Focus element reported as [" + currElement.nodeName + "] in msiEditorOnFocus; changeActiveEditor is [";
    if (changeActiveEditor)
      logString += "true";
    else
      logString += "false";
    msiKludgeLogString(logString + "].\n");
//End logging stuff only
  }

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
                    "_blank", "chrome,close,modal,titlebar");
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


//function openTeX()
//{
//  var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
//  fp.init(window, "Open TeX File", nsIFilePicker.modeOpen);
//
//  //SetFilePickerDirectory(fp, "html");
//
//  // When loading into Composer, direct user to prefer HTML files and text files,
//  //   so we call separately to control the order of the filter list
//  //fp.appendFilters(nsIFilePicker.filterHTML);
//  //fp.appendFilters(nsIFilePicker.filterText);
//  fp.appendFilters(nsIFilePicker.filterAll);
//
//  /* doesn't handle *.shtml files */
//  try {
//    fp.show();
//    /* need to handle cancel (uncaught exception at present) */
//  }
//  catch (ex) {
//    dump("filePicker.chooseInputFile threw an exception\n");
//  }
//
//  /* This checks for already open window and activates it... 
//   * note that we have to test the native path length
//   *  since file.URL will be "file:///" if no filename picked (Cancel button used)
//   */
//  if (fp.file && fp.file.path.length > 0) {
//    //SaveFilePickerDirectory(fp, "html");
//    //editPage(fp.fileURL.spec, window, false);
//    
//    var infile =  fp.file.path;
//    dump("Open Tex: " + infile);
//    var end = infile.lastIndexOf(".");
//    var outfile = infile.substring(0, end) + ".xhtml";
//    dump("\nOutput file: " + outfile);
//    
//    // run tex.exe
//    
//    try {
//      var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
//
//      var exe = dsprops.get("CurProcD", Components.interfaces.nsIFile);
//      exe.append("pretex.exe");
//
//      var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
//      theProcess.init(exe);
//      
//      var dataDir = dsprops.get("CurProcD", Components.interfaces.nsIFile);
//      dataDir.append("ptdata");
//
//      theProcess.run(true, ['-i'+dataDir.target, '-flatex2xml.tex', infile, outfile], 4, {});
//    } catch (ex) {
//         dump("\nUnable to open TeX:\n");
//         dump(ex);
//    }      
//    
//    editPage("file:///" + outfile.replace(/\\/g,"/"), window, true);
//  }                       
//}
  
  
//function exportTeX()
//{
//   dump("\nExport TeX\n");
//   
//   var docUrl = GetDocumentUrl();
//   dump('\nThis doc url = ' + docUrl);
//
//   var scheme, filename;
//   if (docUrl && !IsUrlAboutBlank(docUrl))
//   {
//     scheme = GetScheme(docUrl);
//     filename = GetFilename(docUrl);
//   }
//   dump('\nThis doc = ' + filename);
//
//   if (filename.length < 0)
//      return;
//      
//   var fp = Components.classes["@mozilla.org/filepicker;1"].createInstance(nsIFilePicker);
//   fp.init(window, "Export TeX File", nsIFilePicker.modeSave);
//
//   fp.appendFilters(nsIFilePicker.filterAll);
//
//   /* doesn't handle *.shtml files */
//   try {
//     fp.show();
//     /* need to handle cancel (uncaught exception at present) */
//   }
//   catch (ex) {
//     dump("filePicker threw an exception\n");
//   }
//   
//   if (fp.file && fp.file.path.length > 0) {
//    
//      var exportfile =  fp.file.path;
//      dump("\nExport Tex: " + exportfile);
//    
//      // run saxon.exe
//      //    saxon thisdoc.xml swp.xsl >exportfile.tex
//    
//      try {
//        var dsprops = Components.classes["@mozilla.org/file/directory_service;1"].createInstance(Components.interfaces.nsIProperties);
//
//        var exe = dsprops.get("CurProcD", Components.interfaces.nsIFile);
//        exe.append("runsax.bat");
//
//        var theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
//        theProcess.init(exe);
//      
//        var dataDir = dsprops.get("CurProcD", Components.interfaces.nsIFile);
//        dataDir.append("todata");
//        
//        var xslfile = "swp.xsl";
//        dump('\ncmdline args = ' + filename + ' ' + dataDir.target + '/' + xslfile +' ' + '>'+exportfile);
//        theProcess.run(true, [docUrl, exportfile], 2, {});
//      } catch (ex) {
//           dump("\nUnable to run saxon:\n");
//           dump(ex);
//      }      
//
//  } 
//}
  

//function initializeAutoCompleteStringArrayForEditor(theEditor)
//
//{ 
//  dump("===> initializeAutoCompleteStringArray\n");
//                             
//  var stringArraySearch = Components.classes["@mozilla.org/autocomplete/search;1?name=stringarray"].getService(Components.interfaces.nsIAutoCompleteSearchStringArray);
//  stringArraySearch.editor = theEditor;
//}



//// handle events on prince-specific elements here, or call the default goDoCommand() 
//function goDoPrinceCommand (cmdstr, element) 
//{
//   if ((element.localName.toLowerCase() == "img") && (element.getAttribute("msigraph") == "true"))
//     { graphClickEvent(cmdstr);
//     }
//   else 
//     { goDoCommand(cmdstr);  
//     }
//}
