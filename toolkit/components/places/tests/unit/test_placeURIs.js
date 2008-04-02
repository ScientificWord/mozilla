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
 * The Original Code is Bug 376798 unit test code.
 *
 * The Initial Developer of the Original Code is Mozilla Corporation
 * Portions created by the Initial Developer are Copyright (C) 2007
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Asaf Romano <mano@mozilla.com>
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


// Get history service
try {
  var histsvc = Cc["@mozilla.org/browser/nav-history-service;1"].getService(Ci.nsINavHistoryService);
} catch(ex) {
  do_throw("Could not get history service\n");
} 

// main
function run_test() {
  // XXX Full testing coverage for QueriesToQueryString and
  // QueryStringToQueries

  const NHQO = Ci.nsINavHistoryQueryOptions;
  // Bug 376798
  var query = histsvc.getNewQuery();
  query.setFolders([1],1);
  do_check_eq(histsvc.queriesToQueryString([query], 1, histsvc.getNewQueryOptions()),
              "place:folder=1");

  // Bug 378828
  var options = histsvc.getNewQueryOptions();
  options.sortingAnnotation = "test anno";
  options.sortingMode = NHQO.SORT_BY_ANNOTATION_DESCENDING;
  var placeURI =
    "place:folder=1&sort=" + NHQO.SORT_BY_ANNOTATION_DESCENDING +
    "&sortingAnnotation=test%20anno";
  do_check_eq(histsvc.queriesToQueryString([query], 1, options),
              placeURI);
  var options = {};
  histsvc.queryStringToQueries(placeURI, { }, {}, options);
  do_check_eq(options.value.sortingAnnotation, "test anno");
  do_check_eq(options.value.sortingMode, NHQO.SORT_BY_ANNOTATION_DESCENDING);
}
