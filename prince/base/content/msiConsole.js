function msiConsoleLoad() {
  top.controllers.insertControllerAt(0, CommandController);
  var data = window.arguments[0];
  window.consoleIntervalId = window.setInterval(msiRefreshConsole, 1E3);
  var contentFrame = document.getElementById("contentFrame");
  if (!contentFrame) return;
  var consoleElement = contentFrame.contentDocument.getElementById("console");
  contentFrame.makeEditable("html", false);
  msiRefreshConsole();
}

function msiConsoleUnload() {
  if (window.consoleIntervalId) {
    window.clearInterval(window.consoleIntervalId);
    window.consoleIntervalId = null;
  }
  cleanUp(window.arguments[0]);
}
window.onload = msiConsoleLoad;
window.onunload = msiConsoleUnload;

function cleanUp(data) {
  if (data.clean !== true) {
    data.stdin.close();
    data.pipetransport.terminate();
    data.clean = true;
  }
}

function msiRefreshConsole() {
  var data = window.arguments[0];
  if (data.pipeconsole.hasNewData()) {
    var contentFrame = document.getElementById("contentFrame");
    if (!contentFrame) return false;
    var editor = msiGetEditor(contentFrame);
    if (!editor) {
      window.close();
      return false;
    }
    var consoleElement = contentFrame.contentDocument.getElementById("console");
    consoleElement.firstChild.data = data.pipeconsole.getData();
    if (!contentFrame.mouseDownState) editor.selectionController.completeMove(true, false);
  }
  if (!data.pipetransport.isAttached()) {
    cleanUp(data);
    window.close();
  }
  return true;
}
function msiConsoleCopy() {
  var selText = msiConsoleGetSelectionStr();
  if (selText) {
    var clipHelper = Components.classes["@mozilla.org/widget/clipboardhelper;1"].createInstance(Components.interfaces.nsIClipboardHelper);
    clipHelper.copyString(selText);
  }
  return true;
}
function msiConsoleGetSelectionStr() {
  try {
    var contentFrame = document.getElementById("contentFrame");
    var sel = contentFrame.getSelection();
    return sel.toString();
  } catch (ex) {
    return "";
  }
}

function isItemSelected() {
  return msiConsoleGetSelectionStr() !== "";
}

function UpdateCopyMenu() {
  goUpdateCommand("cmd_copy");
}
var CommandController = {
  isCommandEnabled: function(aCommand) {
    if (aCommand === "cmd_copy") return isItemSelected();
    return false;
  },
  supportsCommand: function(aCommand) {
    return aCommand === "cmd_copy";
  },
  doCommand: function(aCommand) {
    if (aCommand === "cmd_copy") msiConsoleCopy();
  },
  onEvent: function(aEvent) {}
};
var inputstring = "";
var bufferContents = "";

function doKeyPress(event) {
  dump(event + "\n");
  var data = window.arguments[0];
  if (event.charCode || event.keyCode == 13 || event.keyCode == 8) {
    var contentFrame = document.getElementById("contentFrame");
    if (!contentFrame) return;
    var consoleElement = contentFrame.contentDocument.getElementById("console");
    if (bufferContents.length === 0) bufferContents = consoleElement.firstChild.data;
    if (event.charCode) inputstring += String.fromCharCode(event.charCode);
    else if (event.keyCode == 8) inputstring = inputstring.substring(0, inputstring.length - 1);
    else {
      data.stdin.write(inputstring + String.fromCharCode(event.keyCode), inputstring.length + 1);
      inputstring += "\n";
    }
    consoleElement.firstChild.data = bufferContents + inputstring;
    if (event.keycode == 13) {
      inputstring = "";
      bufferContents = "";
    }
    event.preventDefault();
  }
};
