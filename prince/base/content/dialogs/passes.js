
var theProcess;
var passData;
var sentinel;
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
     if (sentinel.exists())
     {
       if (++passData.passCounter < passData.passCount)
       {
         setProgressStatement(false);
         sentinel.remove(false);
         theProcess.run(false, passData.args, passData.args.length);
       }
       else{
         setProgressStatement(true);
         document.getElementById("passesDlg").cancelDialog();
       }
     }
   } 
 }


function Init()
{
  
  passData = window.arguments[0];
  passData.passCounter = 0;
  sentinel = Components.classes["@mozilla.org/file/local;1"].
      createInstance(Components.interfaces.nsILocalFile);
  sentinel.initWithPath(passData.outputDir);
  sentinel.append("sentinel");
  if (sentinel.exists()) sentinel.remove(false);
  theProcess = Components.classes["@mozilla.org/process/util;1"].createInstance(Components.interfaces.nsIProcess);
  theProcess.init(passData.file);
  Components.utils.reportError("in Init\n");
  // set up the first pass
  setProgressStatement(false);
  theProcess.run(false, passData.args, passData.args.length);
  timer.initWithCallback( timerCallback, 250, 1);
}

function setProgressStatement(done)
{
  if (done)
  {
    progressStatement="Done!";
  }
  else
  {
    var progressStatement = document.getElementById("passStatusTemplate").value;
    progressStatement = progressStatement.replace("#n", (Number(passData.passCounter) + 1));
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
  Components.utils.reportError("in OnCancel\n");
  SaveWindowLocation();
//  window.close();
  return true;
}


function newPasses()
{
  passData.passCount = document.getElementById("numpasses").value;
  setProgressStatement();
}