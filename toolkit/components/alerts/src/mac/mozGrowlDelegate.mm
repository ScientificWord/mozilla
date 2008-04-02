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
 * The Original Code is Growl implementation of nsIAlertsService.
 *
 * The Initial Developer of the Original Code is
 *   Shawn Wilsher <me@shawnwilsher.com>.
 * Portions created by the Initial Developer are Copyright (C) 2006-2007
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

#import "mozGrowlDelegate.h"

#include "nsIObserver.h"
#include "nsStringAPI.h"
#include "nscore.h"
#include "nsCOMPtr.h"
#include "nsObjCExceptions.h"
#include "nsServiceManagerUtils.h"
#include "nsIXULAppInfo.h"
#include "nsIStringBundle.h"

@implementation mozGrowlDelegate

- (id) init
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  if ((self = [super init])) {
    mKey = 0;
    mDict = [[NSMutableDictionary dictionaryWithCapacity: 8] retain];
    
    mNames   = [[NSMutableArray alloc] init];
    mEnabled = [[NSMutableArray alloc] init];
  
    nsresult rv;
    nsCOMPtr<nsIStringBundleService> bundleService =
      do_GetService("@mozilla.org/intl/stringbundle;1", &rv);

    if (NS_SUCCEEDED(rv)) {
      nsCOMPtr<nsIStringBundle> bundle;
      rv = bundleService->CreateBundle(GROWL_STRING_BUNDLE_LOCATION,
                                       getter_AddRefs(bundle));
      
      if (NS_SUCCEEDED(rv)) {
        nsString text;
        rv = bundle->GetStringFromName(NS_LITERAL_STRING("general").get(),
                                       getter_Copies(text));
        
        if (NS_SUCCEEDED(rv)) {
          NSString *s = [NSString stringWithCharacters: text.BeginReading()
                                                length: text.Length()];
          [mNames addObject: s];
          [mEnabled addObject: s];

          return self;
        }
      }
    }

    // Fallback
    [mNames addObject: @"General Notification"];
    [mEnabled addObject: @"General Notification"];
  }

  return self;

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}

- (void) dealloc
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  [mDict release];

  [mNames release];
  [mEnabled release];

  [super dealloc];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

- (void) addNotificationNames:(NSArray*)aNames
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  [mNames addObjectsFromArray: aNames];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

- (void) addEnabledNotifications:(NSArray*)aEnabled
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  [mEnabled addObjectsFromArray: aEnabled];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

+ (void) notifyWithName:(const nsAString&)aName
                  title:(const nsAString&)aTitle
            description:(const nsAString&)aText
               iconData:(NSData*)aImage
                    key:(PRUint32)aKey
                 cookie:(const nsAString&)aCookie
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  NS_ASSERTION(aName.Length(), "No name specified for the alert!");
  
  NSDictionary* clickContext = nil;
  if (aKey) {
    clickContext = [NSDictionary dictionaryWithObjectsAndKeys:
      [NSNumber numberWithUnsignedInt: aKey],
      OBSERVER_KEY,
      [NSArray arrayWithObject:
        [NSString stringWithCharacters: aCookie.BeginReading()
                                length: aCookie.Length()]],
      COOKIE_KEY,
      nil];
  }

  [GrowlApplicationBridge
     notifyWithTitle: [NSString stringWithCharacters: aTitle.BeginReading()
                                              length: aTitle.Length()]
         description: [NSString stringWithCharacters: aText.BeginReading()
                                              length: aText.Length()]
    notificationName: [NSString stringWithCharacters: aName.BeginReading()
                                              length: aName.Length()]
            iconData: aImage
            priority: 0
            isSticky: NO
        clickContext: clickContext];

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

- (PRUint32) addObserver:(nsIObserver*)aObserver
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_RETURN;

  NS_ADDREF(aObserver);  // We now own a reference to this!

  mKey++;
  [mDict setObject: [NSValue valueWithPointer: aObserver]
            forKey: [NSNumber numberWithUnsignedInt: mKey]];
  return mKey;

  NS_OBJC_END_TRY_ABORT_BLOCK_RETURN(0);
}

- (NSDictionary *) registrationDictionaryForGrowl
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  return [NSDictionary dictionaryWithObjectsAndKeys:
           mNames, GROWL_NOTIFICATIONS_ALL,
           mEnabled, GROWL_NOTIFICATIONS_DEFAULT,
           nil];

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}

- (NSString*) applicationNameForGrowl
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK_NIL;

  nsresult rv;

  nsCOMPtr<nsIXULAppInfo> appInfo =
    do_GetService("@mozilla.org/xre/app-info;1", &rv);
  NS_ENSURE_SUCCESS(rv, nil);

  nsCAutoString appName;
  rv = appInfo->GetName(appName);
  NS_ENSURE_SUCCESS(rv, nil);

  nsAutoString name = NS_ConvertUTF8toUTF16(appName);
  return [NSString stringWithCharacters: name.BeginReading()
                                 length: name.Length()];

  NS_OBJC_END_TRY_ABORT_BLOCK_NIL;
}

- (void) growlNotificationTimedOut:(id)clickContext
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  NS_ASSERTION([clickContext valueForKey: OBSERVER_KEY] != nil,
               "OBSERVER_KEY not found!");
  NS_ASSERTION([clickContext valueForKey: COOKIE_KEY] != nil,
               "COOKIE_KEY not found!");

  nsIObserver* observer = static_cast<nsIObserver*>
                                     ([[mDict objectForKey: [clickContext valueForKey: OBSERVER_KEY]]
      pointerValue]);
  [mDict removeObjectForKey: [clickContext valueForKey: OBSERVER_KEY]];
  NSString* cookie = [[clickContext valueForKey: COOKIE_KEY] objectAtIndex: 0];

  if (observer) {
    nsAutoString tmp;
    tmp.SetLength([cookie length]);
    [cookie getCharacters:tmp.BeginWriting()];

    observer->Observe(nsnull, "alertfinished", tmp.get());

    NS_RELEASE(observer);
  }

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

- (void) growlNotificationWasClicked:(id)clickContext
{
  NS_OBJC_BEGIN_TRY_ABORT_BLOCK;

  NS_ASSERTION([clickContext valueForKey: OBSERVER_KEY] != nil,
               "OBSERVER_KEY not found!");
  NS_ASSERTION([clickContext valueForKey: COOKIE_KEY] != nil,
               "COOKIE_KEY not found!");

  nsIObserver* observer = static_cast<nsIObserver*>
                                     ([[mDict objectForKey: [clickContext valueForKey: OBSERVER_KEY]]
      pointerValue]);
  [mDict removeObjectForKey: [clickContext valueForKey: OBSERVER_KEY]];
  NSString* cookie = [[clickContext valueForKey: COOKIE_KEY] objectAtIndex: 0];

  if (observer) {
    nsAutoString tmp;
    tmp.SetLength([cookie length]);
    [cookie getCharacters:tmp.BeginWriting()];

    observer->Observe(nsnull, "alertclickcallback", tmp.get());
    observer->Observe(nsnull, "alertfinished", tmp.get());

    NS_RELEASE(observer);
  }

  NS_OBJC_END_TRY_ABORT_BLOCK;
}

@end
