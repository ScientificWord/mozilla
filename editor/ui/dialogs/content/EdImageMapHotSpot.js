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
 * The Original Code is Mozilla Communicator client code, released
 * March 31, 1998.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998-2000
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Dan Haddix
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either of the GNU General Public License Version 2 or later (the "GPL"),
 * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
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

// dialog initialization code
function Startup()
{
  gDialog.urlInput = document.getElementById("urlInput");
  gDialog.targetInput = document.getElementById("targetInput");
  gDialog.altInput = document.getElementById("altInput");
  gDialog.commonInput = document.getElementById("commonInput");

  gDialog.hsHref = window.arguments[0].getAttribute("hsHref");
  if (gDialog.hsHref != '')
    gDialog.urlInput.value = gDialog.hsHref;

  gDialog.hsAlt = window.arguments[0].getAttribute("hsAlt");
  if (gDialog.hsAlt != '')
    gDialog.altInput.value = gDialog.hsAlt;

  gDialog.hsTarget = window.arguments[0].getAttribute("hsTarget");
  if (gDialog.hsTarget != ''){
    gDialog.targetInput.value = gDialog.hsTarget;
    len = gDialog.commonInput.length;
    for (i=0; i<len; i++){
      if (gDialog.hsTarget == gDialog.commonInput.options[i].value)
        gDialog.commonInput.options[i].selected = "true";
    }
  }

  SetTextboxFocus(gDialog.urlInput);

  SetWindowLocation();
}

function onAccept()
{
  dump(window.arguments[0].id+"\n");
  window.arguments[0].setAttribute("hsHref", gDialog.urlInput.value);
  window.arguments[0].setAttribute("hsAlt", gDialog.altInput.value);
  window.arguments[0].setAttribute("hsTarget", gDialog.targetInput.value);

  SaveWindowLocation();

  window.close();
}

function changeTarget() {
  gDialog.targetInput.value=gDialog.commonInput.value;
}

function chooseFile()
{
  // Get a local file, converted into URL format

  fileName = GetLocalFileURL("html");
  if (fileName && fileName != "") {
    gDialog.urlInput.value = fileName;
  }

  // Put focus into the input field
  SetTextboxFocus(gDialog.urlInput);
}
