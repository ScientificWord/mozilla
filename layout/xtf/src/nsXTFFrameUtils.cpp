/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
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
 * The Original Code is the Mozilla XTF project.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Brian Ryner <bryner@brianryner.com>
 *   Alex Fritze <alex@croczilla.com>
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

#include "nsXTFFrameUtils.h"
#include "nsIFrame.h"
#include "nsIXTFVisualWrapperPrivate.h"
#include "nsIDOMElement.h"

nsIFrame*
nsXTFFrameUtils::GetContentInsertionFrame(nsIFrame *aFrame)
{
  nsCOMPtr<nsIContent> content =
    nsXTFFrameUtils::GetContentInsertionNode(aFrame);

  NS_ASSERTION(content, "element not implementing nsIContent!?");

  return aFrame->GetPresContext()->PresShell()->GetPrimaryFrameFor(content);
}


already_AddRefed<nsIContent>
nsXTFFrameUtils::GetContentInsertionNode(nsIFrame *aFrame)
{
  nsCOMPtr<nsIXTFVisualWrapperPrivate> visual =
    do_QueryInterface(aFrame->GetContent());
  NS_ASSERTION(visual, "huh? associated content not implementing nsIXTFVisualWrapperPrivate");

  nsCOMPtr<nsIDOMElement> childInsertionPoint;
  visual->GetInsertionPoint(getter_AddRefs(childInsertionPoint));
  if (!childInsertionPoint) return nsnull; // we don't take visual child content

  nsIContent *content = nsnull;
  CallQueryInterface(childInsertionPoint, &content);
  return content;
}

