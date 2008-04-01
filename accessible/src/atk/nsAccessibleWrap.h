/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* vim:expandtab:shiftwidth=4:tabstop=4:
 */
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
 * Sun Microsystems, Inc.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Bolian Yin (bolian.yin@sun.com)
 *   John Sun (john.sun@sun.com)
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

#ifndef __NS_ACCESSIBLE_WRAP_H__
#define __NS_ACCESSIBLE_WRAP_H__

#include "nsCOMPtr.h"
#include "nsAccessible.h"
#include "prlog.h"

#ifdef PR_LOGGING
#define MAI_LOGGING
#endif /* #ifdef PR_LOGGING */

struct _AtkObject;
typedef struct _AtkObject AtkObject;

enum AtkProperty {
  PROP_0,           // gobject convention
  PROP_NAME,
  PROP_DESCRIPTION,
  PROP_PARENT,      // ancestry has changed
  PROP_ROLE,
  PROP_LAYER,
  PROP_MDI_ZORDER,
  PROP_TABLE_CAPTION,
  PROP_TABLE_COLUMN_DESCRIPTION,
  PROP_TABLE_COLUMN_HEADER,
  PROP_TABLE_ROW_DESCRIPTION,
  PROP_TABLE_ROW_HEADER,
  PROP_TABLE_SUMMARY,
  PROP_LAST         // gobject convention
};

struct AtkPropertyChange {
  PRInt32 type;     // property type as listed above
  void *oldvalue;
  void *newvalue;
};

class MaiHyperlink;

/**
 * nsAccessibleWrap, and its descendents in atk directory provide the
 * implementation of AtkObject.
 */
class nsAccessibleWrap: public nsAccessible
{
public:
    nsAccessibleWrap(nsIDOMNode*, nsIWeakReference *aShell);
    virtual ~nsAccessibleWrap();
    void ShutdownAtkObject();
    NS_IMETHOD Shutdown();

#ifdef MAI_LOGGING
    virtual void DumpnsAccessibleWrapInfo(int aDepth) {}
    static PRInt32 mAccWrapCreated;
    static PRInt32 mAccWrapDeleted;
#endif

    // return the atk object for this nsAccessibleWrap
    NS_IMETHOD GetNativeInterface(void **aOutAccessible);
    NS_IMETHOD FireAccessibleEvent(nsIAccessibleEvent *aEvent);

    AtkObject * GetAtkObject(void);
    static AtkObject * GetAtkObject(nsIAccessible * acc);

    PRBool IsValidObject();
    
    // get/set the MaiHyperlink object for this nsAccessibleWrap
    MaiHyperlink* GetMaiHyperlink(PRBool aCreate = PR_TRUE);
    void SetMaiHyperlink(MaiHyperlink* aMaiHyperlink);

    static const char * ReturnString(nsAString &aString) {
      static nsCString returnedString;
      returnedString = NS_ConvertUTF16toUTF8(aString);
      return returnedString.get();
    }

protected:
    nsresult FireAtkStateChangeEvent(nsIAccessibleEvent *aEvent,
                                     AtkObject *aObject);
    nsresult FireAtkTextChangedEvent(nsIAccessibleEvent *aEvent,
                                     AtkObject *aObject);
    nsresult FireAtkPropChangedEvent(nsIAccessibleEvent *aEvent,
                                     AtkObject *aObject);
    nsresult FireAtkShowHideEvent(nsIAccessibleEvent *aEvent,
                                  AtkObject *aObject, PRBool aIsAdded);

    AtkObject *mAtkObject;

private:
    PRUint16 CreateMaiInterfaces(void);
};

#endif /* __NS_ACCESSIBLE_WRAP_H__ */
