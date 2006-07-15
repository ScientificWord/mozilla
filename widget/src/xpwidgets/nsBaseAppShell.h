/* -*- Mode: c++; tab-width: 2; indent-tabs-mode: nil; -*- */
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
 * The Original Code is Widget code.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Darin Fisher <darin@meer.net> (original author)
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

#ifndef nsBaseAppShell_h__
#define nsBaseAppShell_h__

#include "nsIAppShell.h"
#include "nsIThreadInternal.h"
#include "nsIObserver.h"
#include "nsIRunnable.h"
#include "nsCOMPtr.h"
#include "prinrval.h"

/**
 * A singleton that manages the UI thread's event queue.  Subclass this class
 * to enable platform-specific event queue support.
 */
class nsBaseAppShell : public nsIAppShell, public nsIThreadObserver,
                       public nsIObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIAPPSHELL
  NS_DECL_NSITHREADOBSERVER
  NS_DECL_NSIOBSERVER

  nsBaseAppShell();

protected:
  virtual ~nsBaseAppShell() {}

  /**
   * This method is called by subclasses when the app shell singleton is
   * instantiated.
   */
  nsresult Init();

  /**
   * Called by subclasses from a native event. See ScheduleNativeEventCallback.
   */
  void NativeEventCallback();

  /**
   * Implemented by subclasses.  Invoke NativeEventCallback from a native
   * event.  This method may be called on any thread.
   */
  virtual void ScheduleNativeEventCallback() = 0;

  /**
   * Implemented by subclasses.  Process the next native event.  Only wait for
   * the next native event if mayWait is true.  This method is only called on
   * the main application thread.
   *
   * @param mayWait
   *   If "true", then this method may wait if necessary for the next available
   *   native event.  DispatchNativeEvent may be called to unblock a call to
   *   ProcessNextNativeEvent that is waiting.
   * @return
   *   This method returns "true" if a native event was processed.
   */
  virtual PRBool ProcessNextNativeEvent(PRBool mayWait) = 0;

private:
  PRBool DoProcessNextNativeEvent(PRBool mayWait);

  nsCOMPtr<nsIRunnable> mDummyEvent;
  PRInt32 mFavorPerf;
  PRInt32 mNativeEventPending;
  PRIntervalTime mStarvationDelay;
  PRIntervalTime mSwitchTime;
  PRIntervalTime mLastNativeEventTime;
  PRPackedBool mRunWasCalled;
  PRPackedBool mExiting;
  PRPackedBool mProcessingNextNativeEvent;
};

#endif // nsBaseAppShell_h__
