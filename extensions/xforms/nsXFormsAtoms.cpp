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
 * The Original Code is Mozilla XForms support.
 *
 * The Initial Developer of the Original Code is
 * IBM Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2004
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Brian Ryner <bryner@brianryner.com>
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

#include "nsXFormsAtoms.h"
#include "nsMemory.h"

nsIAtom* nsXFormsAtoms::src;
nsIAtom* nsXFormsAtoms::bind;
nsIAtom* nsXFormsAtoms::type;
nsIAtom* nsXFormsAtoms::readonly;
nsIAtom* nsXFormsAtoms::required;
nsIAtom* nsXFormsAtoms::relevant;
nsIAtom* nsXFormsAtoms::calculate;
nsIAtom* nsXFormsAtoms::constraint;
nsIAtom* nsXFormsAtoms::p3ptype;
nsIAtom* nsXFormsAtoms::modelListProperty;
nsIAtom* nsXFormsAtoms::uploadFileProperty;
nsIAtom* nsXFormsAtoms::messageProperty;
nsIAtom* nsXFormsAtoms::ref;
nsIAtom* nsXFormsAtoms::value;
nsIAtom* nsXFormsAtoms::nodeset;
nsIAtom* nsXFormsAtoms::model;
nsIAtom* nsXFormsAtoms::selected;
nsIAtom* nsXFormsAtoms::appearance;
nsIAtom* nsXFormsAtoms::incremental;
nsIAtom* nsXFormsAtoms::clazz;
nsIAtom* nsXFormsAtoms::deferredBindListProperty;
nsIAtom* nsXFormsAtoms::readyForBindProperty;
nsIAtom* nsXFormsAtoms::fatalError;
nsIAtom* nsXFormsAtoms::isInstanceDocument;
nsIAtom* nsXFormsAtoms::instanceDocumentOwner;
nsIAtom* nsXFormsAtoms::externalMessagesProperty;
nsIAtom* nsXFormsAtoms::deferredEventListProperty;
nsIAtom* nsXFormsAtoms::attrBased;

const nsStaticAtom nsXFormsAtoms::Atoms_info[] = {
  { "src",                      &nsXFormsAtoms::src },
  { "bind",                     &nsXFormsAtoms::bind },
  { "type",                     &nsXFormsAtoms::type },
  { "readonly",                 &nsXFormsAtoms::readonly },
  { "required",                 &nsXFormsAtoms::required },
  { "relevant",                 &nsXFormsAtoms::relevant },
  { "calculate",                &nsXFormsAtoms::calculate },
  { "constraint",               &nsXFormsAtoms::constraint },
  { "p3ptype",                  &nsXFormsAtoms::p3ptype },
  { "ModelListProperty",        &nsXFormsAtoms::modelListProperty },
  { "UploadFileProperty",       &nsXFormsAtoms::uploadFileProperty },
  { "messageProperty",          &nsXFormsAtoms::messageProperty },
  { "ref",                      &nsXFormsAtoms::ref },
  { "value",                    &nsXFormsAtoms::value },
  { "nodeset",                  &nsXFormsAtoms::nodeset },
  { "model",                    &nsXFormsAtoms::model },
  { "selected",                 &nsXFormsAtoms::selected },
  { "appearance",               &nsXFormsAtoms::appearance },
  { "incremental",              &nsXFormsAtoms::incremental },
  { "class",                    &nsXFormsAtoms::clazz },
  { "DeferredBindListProperty", &nsXFormsAtoms::deferredBindListProperty },
  { "ReadyForBindProperty",     &nsXFormsAtoms::readyForBindProperty },
  { "fatalError",               &nsXFormsAtoms::fatalError },
  { "isInstanceDocument",       &nsXFormsAtoms::isInstanceDocument },
  { "instanceDocumentOwner",    &nsXFormsAtoms::instanceDocumentOwner },
  { "ExternalMessagesProperty", &nsXFormsAtoms::externalMessagesProperty },
  { "DeferredEventListProperty",&nsXFormsAtoms::deferredEventListProperty },
  { "attrBased",                &nsXFormsAtoms::attrBased }
};

void
nsXFormsAtoms::InitAtoms()
{
  NS_RegisterStaticAtoms(Atoms_info, NS_ARRAY_LENGTH(Atoms_info));
}
