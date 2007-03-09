/*
The contents of this file are subject to the Mozilla Public
License Version 1.1 (the "MPL"); you may not use this file
except in compliance with the MPL. You may obtain a copy of
the MPL at http://www.mozilla.org/MPL/

Software distributed under the MPL is distributed on an "AS
IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
implied. See the MPL for the specific language governing
rights and limitations under the MPL.

The Original Code is Enigmail.

The Initial Developer of the Original Code is Ramalingam Saravanan.
Portions created by Ramalingam Saravanan <svn@xmlterm.org> are
Copyright (C) 2002 Ramalingam Saravanan. All Rights Reserved.

Contributor(s):
Patrick Brunschwig <patrick.brunschwig@gmx.net>

Alternatively, the contents of this file may be used under the
terms of the GNU General Public License (the "GPL"), in which case
the provisions of the GPL are applicable instead of
those above. If you wish to allow use of your version of this
file only under the terms of the GPL and not to allow
others to use your version of this file under the MPL, indicate
your decision by deleting the provisions above and replace them
with the notice and other provisions required by the GPL.
If you do not delete the provisions above, a recipient
may use your version of this file under either the MPL or the
GPL.
*/

// Initialize msiCommon


function msiConsoleLoad() {
//  DEBUG_LOG("msiConsole.js: msiConsoleLoad\n");

  top.controllers.insertControllerAt(0, CommandController);

  var data = window.arguments[0];
//  if (!data)
//    return;

  // Refresh console every second
  window.consoleIntervalId = window.setInterval(msiRefreshConsole, 1000);
  var contentFrame = document.getElementById("contentFrame");
  if (!contentFrame)
    return;
  var consoleElement = contentFrame.contentDocument.getElementById("console");
  contentFrame.makeEditable("html", false);  
  msiRefreshConsole();
}

function msiConsoleUnload() {
//  DEBUG_LOG("msiConsole.js: msiConsoleUnload\n");

  // Cancel console refresh
  if (window.consoleIntervalId) {
    window.clearInterval(window.consoleIntervalId);
    window.consoleIntervalId = null;
  }
  cleanUp(window.arguments[0]);
}

window.onload = msiConsoleLoad;
window.onunload = msiConsoleUnload;

function cleanUp( data )
{
  if (data.clean != true)
  { 
//    data.pipeconsole.shutDown();
    data.stdin.close();
    data.pipetransport.terminate();
    data.clean = true;
  }
}

function msiRefreshConsole() {
  //DEBUG_LOG("msiConsole.js: msiRefreshConsole():\n");

  var data = window.arguments[0];
  if (data.pipeconsole.hasNewData()) 
  {
//    DEBUG_LOG("msiConsole.js: msiRefreshConsole(): hasNewData\n");

    var contentFrame = document.getElementById("contentFrame");
    if (!contentFrame)
      return false;
    var editor = msiGetEditor(contentFrame);
    if (!editor) {
      window.close();
      return false;
    }

    var consoleElement = contentFrame.contentDocument.getElementById("console");

    consoleElement.firstChild.data = data.pipeconsole.getData();

    if (!contentFrame.mouseDownState)
       editor.selectionController.completeMove(true, false);
  }
  if (!data.pipetransport.isAttached())
  {
    cleanUp(data);
    window.close();
  }
  return true;
}

function msiConsoleCopy()
{
  var selText = msiConsoleGetSelectionStr();

//  DEBUG_LOG("msiConsole.js: msiConsoleCopy: selText='"+selText+"'\n");

  if (selText) {
    var clipHelper = Components.classes["@mozilla.org/widget/clipboardhelper;1"].createInstance(Components.interfaces.nsIClipboardHelper);

    clipHelper.copyString(selText);
  }

  return true;
}

function msiConsoleGetSelectionStr()
{
  try {
    var contentFrame = document.getElementById("contentFrame");

    var sel = contentFrame.getSelection();
    return sel.toString();

  } catch (ex) {
    return "";
  }
}

function isItemSelected()
{
//  DEBUG_LOG("msiConsole.js: isItemSelected\n");
  return msiConsoleGetSelectionStr() != "";
}

function UpdateCopyMenu()
{
//  DEBUG_LOG("msiConsole.js: msiConsoleUpdateCopyMenu\n");
  goUpdateCommand("cmd_copy");
}

var CommandController = 
{
  isCommandEnabled: function (aCommand)
  {
    switch (aCommand) {
      case "cmd_copy":
        return isItemSelected();
      default:
        return false;
    }
  },
  
  supportsCommand: function (aCommand) 
  {
    switch (aCommand) {
      case "cmd_copy":
        return true;
      default:
        return false;
    }
  },
  
  doCommand: function (aCommand)
  {
    switch (aCommand) {
      case "cmd_copy":
        msiConsoleCopy();
        break;
      default:
        break;
    }
  },
  
  onEvent: function (aEvent) 
  {
  }
};

var inputstring="";
var bufferContents = ""

function doKeyPress(event)
{
  dump(event+"\n");
  var data = window.arguments[0];
  if (event.charCode || event.keyCode==13 || event.keyCode==8)
  {
    var contentFrame = document.getElementById("contentFrame");
    if (!contentFrame)
      return;

    var consoleElement = contentFrame.contentDocument.getElementById("console");
    if (bufferContents.length == 0)
      bufferContents = consoleElement.firstChild.data;  
    if (event.charCode)
      inputstring += String.fromCharCode(event.charCode);
    else if (event.keyCode == 8)
      inputstring = inputstring.substring(0, inputstring.length-1);
    else
    {
      data.stdin.write(inputstring+String.fromCharCode(event.keyCode), inputstring.length+1);
      inputstring += "\n"; // add return to end of string
    }
    consoleElement.firstChild.data = bufferContents + inputstring;
    if (event.keycode == 13)
    {
      inputstring = "";
      bufferContents = "";
    }
    event.preventDefault();
  }
}
