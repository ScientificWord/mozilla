
var theProcess;
var theIndexProcess;
var theBibTeXProcess;
var passData;
var compileInfo;
var showdialog;
var previewPDF;
var sentinel = null;
var blocking = false; // set whether blocking or not.
var timer = Components.classes["@mozilla.org/timer;1"]
                    .createInstance(Components.interfaces.nsITimer);

var timerCallback =
{
  timercopy: timer,
  notify: function(timer)
    {
      if (timer && (this.timercopy !== timer)) this.timercopy = timer;
      if (!sentinel)
      {
         if (this.timercopy) this.timercopy.cancel();
         return;
      }
      if (sentinel.exists())
      {
        if (passData.passCounter < passData.passCount)
        {
          if (showdialog) setProgressStatement(false);
          sentinel.remove(false);
          if (passData.runBibTeX)
          {
            theBibTeXProcess.run(blocking, passData.args, passData.args.length);
            passData.runBibTeX = false;
          }
          else if (passData.runMakeIndex)
          {
            theIndexProcess.run(blocking, passData.args, passData.args.length);
            passData.runMakeIndex = false;
          }
          else
          {
            passData.passCounter++;
            theProcess.run(blocking, passData.args, passData.args.length);
          }
        }
        else{
          timer.cancel();
          timer = null;
          if (showdialog) {
            SaveWindowLocation();
            setProgressStatement(true);
    //        document.getElementById("oktocontinue").hidden = false;
            var dlg = document.getElementById("passesDlg");
    //        dlg.getButton("cancel").disabled = true;
    //        top.document.commandDispatcher.focusedWindow.focus();
    //         window.opener.cancelSendMessage = true;
    //         window.close();
          }
          var outputfile = Components.classes["@mozilla.org/file/local;1"].createInstance(Components.interfaces.nsILocalFile);
          outputfile.initWithPath( passData.outputDir );
          var tempOutputfile;
          tempOutputfile = outputfile.clone();
          var leaf = "SWP.pdf";
          outputfile.append("main.pdf"); // outputfile is now main.pdf. This is the result of the compilation.
          tempOutputfile.append(leaf); // this is SWP.pdf which is the new name of main.pdf if there is no collision.
          var n = 0;
          msidump("Leaf is "+leaf+"\n");
          while (tempOutputfile.exists())
          {
            leaf = "SWP"+(n++)+".pdf";
            tempOutputfile = tempOutputfile.parent;
            tempOutputfile.append(leaf);
          }
          // now tempOutputfile's leaf is SWP[n].pdf, and doesn't exist.
          if (outputfile.exists())
          {
            outputfile.moveTo(null, leaf);     // rename main.pdf to SWP[i].pdf
          }
          previewPDF(outputfile);
          if (showdialog) {
//            dlg.getButton('cancel').oncommand();
            dlg.cancelDialog();
          }

        }
      }
    }
 }

function initForSilentCompile(aPassData, aCompileInfo, aShowdialog, callbackFn) // an alternative to using the dialog
{
  passData = aPassData;
  showdialog = aShowdialog;
  compileInfo = aCompileInfo;
  previewPDF = callbackFn;
  passData.passCounter = 1;
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
    bibinputs = prefs.getCharPref("swp.bibtex.dir");
    if (bibinputs.length > 0) {
      passData.args.push('-d');
      passData.args.push(bibinputs);
    }
  }
  Components.utils.reportError("in Init\n");
  theProcess.run(blocking, passData.args, passData.args.length);
  timer.initWithCallback( timerCallback, 250, 1);
}

function Init()
{
  var prefs = GetPrefs();
  var bibinputs;
  var bstinputs;
  passData = window.arguments[0];
  passData.passCounter = 1;
  compileInfo = window.arguments[1];
  showdialog = window.arguments[2];
  previewPDF = window.arguments[3];
  var dlg = document.getElementById("passesDlg");
  dlg.getButton("accept").disabled = true;
  dlg.getButton("cancel").disabled = false;
  // set up the first pass
  setProgressStatement(false);
  document.getElementById("numpasses").value = passData.passCount;
  initForSilentCompile(passData, compileInfo, true, previewPDF);
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
