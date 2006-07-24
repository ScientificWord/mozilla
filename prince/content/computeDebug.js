// Copyright (c) 2004 MacKichan Software, Inc.  All Rights Reserved.

//-----------------------------------------------------------------------------------

function timeReport(domdoc,label)
{
  var time = new Date();
  var timeStr = time.toLocaleString();
  var report = domdoc.createElement("p");
  var text = domdoc.createTextNode(label + timeStr);
  report.appendChild(text);
  return report;
}

function makeTR(domdoc,label,value)
{
  var tr = domdoc.createElement("tr");
  var td = domdoc.createElement("td");
  td.appendChild(domdoc.createTextNode(label));
  tr.appendChild(td);
  td = domdoc.createElement("td");
  td.appendChild(domdoc.createTextNode(value));
  tr.appendChild(td);
  return tr;
}

var mathCount = 0;
var evalCorrect = 0;
var evalWrong = 0;

function testMath(math,cmd)
{
  ++mathCount;
  if (cmd == "Evaluate") {
    doComputeEvaluate(math);
  } else {
    dump("Unknown test command!\n");
  }
}

function doCompare(math,ans)
{
  if (math.localName != ans.localName) {
    return false;
  }
  if (count_children(math) != count_children(ans))
    return false;
  return true;
}

function compareMath(domdoc,math,ans,chk)
{
  if (doCompare(math,ans)) {
    ++evalCorrect;
    ans.parentNode.appendChild(domdoc.createTextNode("Correct"));
  } else {
    ++evalWrong;
    ans.parentNode.appendChild(domdoc.createTextNode("Wrong"));
  }
}

function ComputeEvalTest()
{
  dump("Test Evaluation started.\n");

  var bodyelement = GetBodyElement();
  var editor = GetCurrentEditor();
  var domdoc = editor.document;

  bodyelement.appendChild(domdoc.createElement("hr"));
  bodyelement.appendChild(timeReport(domdoc,"Test started: "));

  var table = domdoc.createElement("table");
  var tbody = domdoc.createElement("tbody");
  tbody.appendChild(makeTR(domdoc,"Title",      GetDocumentTitle()));
  tbody.appendChild(makeTR(domdoc,"URL",        GetDocumentUrl()));
  tbody.appendChild(makeTR(domdoc,"User Agent", navigator.userAgent));
  table.appendChild(tbody);
  bodyelement.appendChild(table);

  // run test
  var p = first_child(bodyelement);
  while (p) {
    if (p.localName == "p") {
      var cmd = p.getAttribute("princeCmd");
      if (!cmd || cmd == "")
        cmd = "Evaluate";
      var res = p.getAttribute("princeResult");
      if (!res || res == "")
        res = "CompareExact";
      var math = first_child(p);
      if (math && math.localName == "math") {
        var ans = node_after(math);
        testMath(math,cmd);
        if (ans) {
          math = first_child(p);
          compareMath(domdoc,math,ans,res);
        }
      }
    }
    p = node_after(p);
  }

  bodyelement.appendChild(domdoc.createElement("br"));
  table = domdoc.createElement("table");
  tbody = domdoc.createElement("tbody");
  tbody.appendChild(makeTR(domdoc,"Expressions seen",    mathCount.toString()));
  tbody.appendChild(makeTR(domdoc,"Evaluations correct", evalCorrect.toString()));
  tbody.appendChild(makeTR(domdoc,"Evaluations wrong",   evalWrong.toString()));
  table.appendChild(tbody);
  bodyelement.appendChild(table);

  var h1 = domdoc.createElement("h1");
  if (evalWrong > 0) {
    h1.appendChild(domdoc.createTextNode("Fail"));
  } else {
    h1.appendChild(domdoc.createTextNode("Pass"));
  }
  bodyelement.appendChild(h1);

  bodyelement.appendChild(timeReport(domdoc,"Test finished: "));

  dump("Test Evaluation finished.\n");
}

