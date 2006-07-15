/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 1998
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

#ifndef __SCRIPT_HPP__
#define __SCRIPT_HPP__

#include "plugbase.h"
#include "action.h"

struct ScriptArgumentStruct
{
  DWORD dwArg;
  int iLength;

  ScriptArgumentStruct()
  {
    iLength = 0;
  }

  ~ScriptArgumentStruct()
  {
    if(iLength != 0)
      delete [] (char *)dwArg;
    iLength = 0;
  }
};

struct ScriptItemStruct
{
  NPAPI_Action action;
  ScriptArgumentStruct arg1;
  ScriptArgumentStruct arg2;
  ScriptArgumentStruct arg3;
  ScriptArgumentStruct arg4;
  ScriptArgumentStruct arg5;
  ScriptArgumentStruct arg6;
  ScriptArgumentStruct arg7;
  DWORD dwDelay; // im milliseconds

  ScriptItemStruct()
  {
  }

  ~ScriptItemStruct()
  {
  }
};

struct ScriptItemListElement
{
  ScriptItemStruct * psis;
  ScriptItemListElement * pPrev;
  ScriptItemListElement * pNext;

  ScriptItemListElement()
  {
    psis = NULL;
    pPrev = NULL;
    pNext = NULL;
  }

  ~ScriptItemListElement()
  {
    if(psis != NULL)
      delete psis;
  }
};

class CScriptItemList
{
public:
  ScriptItemListElement * m_pFirst;
  ScriptItemListElement * m_pLast;
  int m_iCount;

public:
  CScriptItemList();
  ~CScriptItemList();

  int add(ScriptItemStruct * psis);
};

#endif // __SCRIPT_HPP__
