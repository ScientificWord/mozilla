
var theProcess;
var passData;
var timer = Components.classes["@mozilla.org/timer;1"]
                    .createInstance(Components.interfaces.nsITimer);

var timerCallback = 
{ notify: function(timer)
   { 
//     Components.utils.reportError("exitValue = "+theProcess.exitValue+"\n");
//     if (theProcess && theProcess.exitValue < 0) return;
     // if we get here, the process is termination
     // if (exitValue > 0) there was an error
     //Components.utils.reportError("Exception: "+e.message);
     if (++passData.passCounter < passData.passCount)
     {
       doIteration(timer);
     }
     else document.getElementById("dialog").cancelDialog();
   } 
 }


function doIteration(timer)
{
  theProcess = null;
  theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
  theProcess.init(passData.file);
  var args = passData.args;
  setProgressStatement()
  theProcess.run(true,  passData.args,  passData.args.length);
}



function Init()
{
  Components.utils.reportError("in Init\n");
  passData = window.arguments[0];
  passData.passCounter = 0;
  // set up the first pass
  setProgressStatement();
  timer.initWithCallback( timerCallback, 250, 1);
}

function setProgressStatement()
{
  var progressStatement = document.getElementById("passStatusTemplate").value;
  progressStatement = progressStatement.replace("#n", (Number(passData.passCounter) + 1));
  progressStatement = progressStatement.replace("#m", (Number(passData.passCount)));
  document.getElementById("passStatus").value = progressStatement;
}


function onCancel()
{
  if (timer) timer.cancel();
  timer=null;
  Components.utils.reportError("in OnCancel\n");
  SaveWindowLocation();
  return true;
}


function newPasses()
{
  passData.passCount = document.getElementById("numpasses").value;
  setProgressStatement();
}