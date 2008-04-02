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
 * The Original Code is XPCOM unit tests.
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

const Cc = Components.classes;
const Ci = Components.interfaces;
const Cr = Components.results;
const CC = Components.Constructor;

var Pipe = CC("@mozilla.org/pipe;1", "nsIPipe", "init");

function run_test()
{
  test_not_initialized();
  test_ends_are_threadsafe();
}

function test_not_initialized()
{
  var p = Cc["@mozilla.org/pipe;1"]
            .createInstance(Ci.nsIPipe);
  try
  {
    var dummy = p.outputStream;
    throw Cr.NS_ERROR_FAILURE;
  }
  catch (e)
  {
    if (e.result != Cr.NS_ERROR_NOT_INITIALIZED)
      do_throw("using a pipe before initializing it should throw NS_ERROR_NOT_INITIALIZED");
  }
}

function test_ends_are_threadsafe()
{
  var p, is, os;

  p = new Pipe(true, true, 1024, 1, null);
  is = p.inputStream.QueryInterface(Ci.nsIClassInfo);
  os = p.outputStream.QueryInterface(Ci.nsIClassInfo);
  do_check_true(Boolean(is.flags & Ci.nsIClassInfo.THREADSAFE));
  do_check_true(Boolean(os.flags & Ci.nsIClassInfo.THREADSAFE));

  p = new Pipe(true, false, 1024, 1, null);
  is = p.inputStream.QueryInterface(Ci.nsIClassInfo);
  os = p.outputStream.QueryInterface(Ci.nsIClassInfo);
  do_check_true(Boolean(is.flags & Ci.nsIClassInfo.THREADSAFE));
  do_check_true(Boolean(os.flags & Ci.nsIClassInfo.THREADSAFE));

  p = new Pipe(false, true, 1024, 1, null);
  is = p.inputStream.QueryInterface(Ci.nsIClassInfo);
  os = p.outputStream.QueryInterface(Ci.nsIClassInfo);
  do_check_true(Boolean(is.flags & Ci.nsIClassInfo.THREADSAFE));
  do_check_true(Boolean(os.flags & Ci.nsIClassInfo.THREADSAFE));

  p = new Pipe(false, false, 1024, 1, null);
  is = p.inputStream.QueryInterface(Ci.nsIClassInfo);
  os = p.outputStream.QueryInterface(Ci.nsIClassInfo);
  do_check_true(Boolean(is.flags & Ci.nsIClassInfo.THREADSAFE));
  do_check_true(Boolean(os.flags & Ci.nsIClassInfo.THREADSAFE));
}
