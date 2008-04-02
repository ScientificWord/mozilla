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
 * The Original Code is MozJSHTTP code.
 *
 * The Initial Developer of the Original Code is
 * Jeff Walden <jwalden+code@mit.edu>.
 * Portions created by the Initial Developer are Copyright (C) 2006
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

// Request handlers may throw exceptions, and those exception should be caught
// by the server and converted into the proper error codes.

var paths =
  [
   "http://localhost:4444/throws/exception",
   "http://localhost:4444/this/file/does/not/exist/and/404s",
   "http://localhost:4444/attempts/404/fails/so/400/fails/so/500s"
  ];
var currPathIndex = 0;

var listener =
  {
    // NSISTREAMLISTENER
    onDataAvailable: function(request, cx, inputStream, offset, count)
    {
      makeBIS(inputStream).readByteArray(count); // required by API
    },
    // NSIREQUESTOBSERVER
    onStartRequest: function(request, cx)
    {
      var ch = request.QueryInterface(Ci.nsIHttpChannel)
                      .QueryInterface(Ci.nsIHttpChannelInternal);

      switch (currPathIndex)
      {
        case 0:
          checkStatusLine(ch, 1, 1, 500, "Internal Server Error");
          break;

        case 1:
          checkStatusLine(ch, 1, 1, 400, "Bad Request");
          break;

        case 2:
          checkStatusLine(ch, 1, 1, 500, "Internal Server Error");
          break;
      }
    },
    onStopRequest: function(request, cx, status)
    {
      do_check_true(Components.isSuccessCode(status));

      switch (currPathIndex)
      {
        case 0:
          break;

        case 1:
          srv.registerErrorHandler(400, throwsException);
          break;

        case 2:
          break;
      }

      if (++currPathIndex == paths.length)
        srv.stop();
      else
        performNextTest();
      do_test_finished();
    },
    // NSISUPPORTS
    QueryInterface: function(aIID)
    {
      if (aIID.equals(Ci.nsIStreamListener) ||
          aIID.equals(Ci.nsIRequestObserver) ||
          aIID.equals(Ci.nsISupports))
        return this;
      throw Cr.NS_ERROR_NO_INTERFACE;
    }
  };

function checkStatusLine(channel, httpMaxVer, httpMinVer, httpCode, statusText)
{
  do_check_eq(channel.responseStatus, httpCode);
  do_check_eq(channel.responseStatusText, statusText);

  var respMaj = {}, respMin = {};
  channel.getResponseVersion(respMaj, respMin);
  do_check_eq(respMaj.value, httpMaxVer);
  do_check_eq(respMin.value, httpMinVer);
}

function performNextTest()
{
  do_test_pending();

  var ch = makeChannel(paths[currPathIndex]);
  ch.asyncOpen(listener, null);
}

var srv;

function run_test()
{
  srv = createServer();

  srv.registerErrorHandler(404, throwsException);
  srv.registerPathHandler("/throws/exception", throwsException);

  srv.start(4444);

  performNextTest();
}

// PATH HANDLERS

// /throws/exception (and also a 404 and 400 error handler)
function throwsException(metadata, response)
{
  throw "this shouldn't cause an exit...";
  do_throw("Not reached!");
}
