
var theProcess;
var theIndexProcess;
var theBibTeXProcess;
var passData;
var sentinel;
var timer = Components.classes["@mozilla.org/timer;1"]
                    .createInstance(Components.interfaces.nsITimer);

var timerCallback = 
{  
  timercopy: timer,
  notify: function(timer)
   { 
     if (timer) timercopy = timer;
     if (!sentinel)
     {
        if (timercopy) timercopy.cancel();
        return;
     } 
     if (sentinel.exists())
     {
       if (passData.passCounter < passData.passCount)
       {
         setProgressStatement(false);
         sentinel.remove(false);
         if (passData.runBibTeX)
         {
           theBibTeXProcess.run(false, passData.args, passData.args.length);
           passData.runBibTeX = false;
         }
         else if (passData.runMakeIndex)
         {
           theIndexProcess.run(false, passData.args, passData.args.length);
           passData.runMakeIndex = false;
         }
         else
         {
           passData.passCounter++;
           theProcess.run(false, passData.args, passData.args.length);
         }
       }
       else{
         timer.cancel();
         timer = null;
         SaveWindowLocation();
         setProgressStatement(true);
         document.getElementById("oktocontinue").hidden = false;
         var dlg = document.getElementById("passesDlg");
         dlg.getButton("accept").disabled = false;
         dlg.getButton("cancel").disabled = true;
         top.document.commandDispatcher.focusedWindow.focus();  
//         window.opener.cancelSendMessage = true;
//         window.close();
       }
     }
   } 
 }


function Init()
{
  
  passData = window.arguments[0];
  passData.passCounter = 1;
  document.getElementById("numpasses").value = passData.passCount;
  sentinel = Components.classes["@mozilla.org/file/local;1"].
      createInstance(Components.interfaces.nsILocalFile);
  sentinel.initWithPath(passData.outputDir);
  sentinel.append("sentinel");
  if (sentinel.exists()) sentinel.remove(false);
  theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
  theProcess.init(passData.file);
  if (passData.runMakeIndex) {
    theIndexProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theIndexProcess.init(passData.indexexe);
  }
  if (passData.runBibTeX)
  {
    theBibTeXProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
    theBibTeXProcess.init(passData.bibtexexe);
  }
  Components.utils.reportError("in Init\n");
  var dlg = document.getElementById("passesDlg");
  dlg.getButton("accept").disabled = true;
  dlg.getButton("cancel").disabled = false;
  // set up the first pass
  setProgressStatement(false);
  theProcess.run(false, passData.args, passData.args.length);
  timer.initWithCallback( timerCallback, 250, 1);
}

function setProgressStatement(done)
{
  var progressStatement = "";
  if (done)
  {
    progressStatement = document.getElementById("doneMessage").value;
    document.documentElement.setAttribute("buttonlabelcancel", document.documentElement.getAttribute("buttonlabelclose"));
    progressStatement="Done!";
  }
  else
  {
    progressStatement = document.getElementById("passStatusTemplate").value;
    progressStatement = progressStatement.replace("#n", (Number(passData.passCounter)));
    progressStatement = progressStatement.replace("#m", (Number(passData.passCount)));
  }
  document.getElementById("passStatus").value = progressStatement;
}


function onCancel()
{
  if (timer) timer.cancel();
  timer=null;
  if (sentinel.exists()) sentinel.remove(false);
  theProcess = null;
  Components.utils.reportError("in onCancel\n");
  SaveWindowLocation();
  top.document.commandDispatcher.focusedWindow.focus();  
  window.close();
  return true;
}


function onAccept()
{
  try {
    if (timer) timer.cancel();
    timer=null;
    if (sentinel && sentinel.exists()) sentinel.remove(false);
    theProcess = null;
    Components.utils.reportError("in onAccept\n");
    SaveWindowLocation();
  }
  catch(e) {}
  return true;
}


function newPasses()
{
  passData.passCount = document.getElementById("numpasses").value;
  setProgressStatement();
}