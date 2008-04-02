/* -*- Mode: Java; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et: */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is httpd.js code.
 *
 * The Initial Developer of the Original Code is
 * Jeff Walden <jwalden+code@mit.edu>.
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

// tests support for server JS-generated pages

const BASE = "http://localhost:4444";

var sjs = do_get_file("netwerk/test/httpserver/test/data/sjs/cgi.sjs");
var srv;
var test;
var tests = [];


/*********************
 * UTILITY FUNCTIONS *
 *********************/

function isException(e, code)
{
  if (e !== code && e.result !== code)
    do_throw("unexpected error: " + e);
}

function bytesToString(bytes)
{
  return bytes.map(function(v) { return String.fromCharCode(v); }).join("");
}

function skipCache(ch)
{
  ch.loadFlags |= Ci.nsIRequest.LOAD_BYPASS_CACHE;
}


/********************
 * DEFINE THE TESTS *
 ********************/

/**
 * Adds the set of tests defined in here, differentiating between tests with a
 * SJS which throws an exception and creates a server error and tests with a
 * normal, successful SJS.
 */
function setupTests(throwing)
{
  const TEST_URL = BASE + "/cgi.sjs" + (throwing ? "?throw" : "");

  //   registerFile with SJS => raw text

  function setupFile(ch)
  {
    srv.registerFile("/cgi.sjs", sjs);
    skipCache(ch);
  }

  function verifyRawText(channel, cx, status, bytes)
  {
    dumpn(channel.originalURI.spec);
    do_check_eq(bytesToString(bytes), fileContents(sjs));
  }

  test = new Test(TEST_URL, setupFile, null, verifyRawText);
  tests.push(test);


  //   add mapping, => interpreted

  function addTypeMapping(ch)
  {
    srv.registerContentType("sjs", "sjs");
    skipCache(ch);
  }

  function checkType(ch, cx)
  {
    if (throwing)
    {
      do_check_false(ch.requestSucceeded);
      do_check_eq(ch.responseStatus, 500);
    }
    else
    {
      do_check_eq(ch.contentType, "text/plain");
    }
  }

  function checkContents(ch, cx, status, data)
  {
    if (!throwing)
      do_check_eq("PASS", bytesToString(data));
  }

  test = new Test(TEST_URL, addTypeMapping, checkType, checkContents);
  tests.push(test);


  //   remove file/type mapping, map containing directory => raw text

  function setupDirectoryAndRemoveType(ch)
  {
    dumpn("removing type mapping");
    srv.registerContentType("sjs", null);
    srv.registerFile("/cgi.sjs", null);
    srv.registerDirectory("/", sjs.parent);
    skipCache(ch);
  }

  test = new Test(TEST_URL, setupDirectoryAndRemoveType, null, verifyRawText);
  tests.push(test);


  //   add mapping, => interpreted
  
  function contentAndCleanup(ch, cx, status, data)
  {
    checkContents(ch, cx, status, data);

    // clean up state we've set up
    srv.registerDirectory("/", null);
    srv.registerContentType("sjs", null);
  }

  test = new Test(TEST_URL, addTypeMapping, checkType, contentAndCleanup);
  tests.push(test);

  // NB: No remaining state in the server right now!  If we have any here,
  //     either the second run of tests (without ?throw) or the two tests
  //     added after the two sets will almost certainly fail.
}


/*****************
 * ADD THE TESTS *
 *****************/

setupTests(true);
setupTests(false);

// Test that when extension-mappings are used, the entire filename cannot be
// treated as an extension -- there must be at least one dot for a filename to
// match an extension.

function init(ch)
{
  // clean up state we've set up
  srv.registerDirectory("/", sjs.parent);
  srv.registerContentType("sjs", "sjs");
  skipCache(ch);
}

function checkNotSJS(ch, cx, status, data)
{
  do_check_neq("FAIL", bytesToString(data));
}

test = new Test(BASE + "/sjs", init, null, checkNotSJS);
tests.push(test);


// One last test: for file mappings, the content-type is determined by the
// extension of the file on the server, not by the extension of the requested
// path.

function setupFileMapping(ch)
{
  srv.registerFile("/script.html", sjs);
}

function onStart(ch, cx)
{
  do_check_eq(ch.contentType, "text/plain");
}

function onStop(ch, cx, status, data)
{
  do_check_eq("PASS", bytesToString(data));
}

test = new Test(BASE + "/script.html", setupFileMapping, onStart, onStop);
tests.push(test);


/*****************
 * RUN THE TESTS *
 *****************/

function run_test()
{
  srv = createServer();

  // Test for a content-type which isn't a field-value
  try
  {
    srv.registerContentType("foo", "bar\nbaz");
    throw "this server throws on content-types which aren't field-values";
  }
  catch (e)
  {
    isException(e, Cr.NS_ERROR_INVALID_ARG);
  }


  // NB: The server has no state at this point -- all state is set up and torn
  //     down in the tests, because we run the same tests twice with only a
  //     different query string on the requests, followed by the oddball
  //     test that doesn't care about throwing or not.

  srv.start(4444);
  runHttpTests(tests, function() { srv.stop(); });
}
