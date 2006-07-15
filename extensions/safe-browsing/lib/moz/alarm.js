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
 * The Original Code is Google Safe Browsing.
 *
 * The Initial Developer of the Original Code is Google Inc.
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Fritz Schneider <fritz@google.com> (original author)
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


// An Alarm fires a callback after a certain amount of time, or at
// regular intervals. It's a convenient replacement for
// setTimeout/Interval when you don't want to bind to a specific
// window.
//
// The ConditionalAlarm is an Alarm that cancels itself if its callback 
// returns a value that type-converts to true.
//
// Example:
//
//  function foo() { alert('hi'); };
//  new G_Alarm(foo, 10*1000);                   // Fire foo in 10 seconds
//  new G_Alarm(foo, 10*1000, true /*repeat*/);  // Fire foo every 10 seconds
//  new G_Alarm(foo, 10*1000, true, 7);          // Fire foo every 10 seconds
//                                               // seven times
//  new G_ConditionalAlarm(foo, 1000, true); // Fire every sec until foo()==true
//
//  // Fire foo every 10 seconds until foo returns true or until it fires seven
//  // times, whichever happens first.
//  new G_ConditionalAlarm(foo, 10*1000, true /*repeating*/, 7);
//
// TODO: maybe pass an isFinal flag to the callback if they opted to
// set maxTimes and this is the last iteration?


/**
 * Set an alarm to fire after a given amount of time, or at specific 
 * intervals.
 *
 * @param callback Function to call when the alarm fires
 * @param delayMS Number indicating the length of the alarm period in ms
 * @param opt_repeating Boolean indicating whether this should fire 
 *                      periodically
 * @param opt_maxTimes Number indicating a maximum number of times to 
 *                     repeat (obviously only useful when opt_repeating==true)
 */
function G_Alarm(callback, delayMS, opt_repeating, opt_maxTimes) {
  this.debugZone = "alarm";
  this.callback_ = callback;
  this.repeating_ = !!opt_repeating;
  this.timer_ = Cc["@mozilla.org/timer;1"].createInstance(Ci.nsITimer);
  var type = opt_repeating ? 
             this.timer_.TYPE_REPEATING_SLACK : 
             this.timer_.TYPE_ONE_SHOT;
  this.maxTimes_ = opt_maxTimes ? opt_maxTimes : null;
  this.nTimes_ = 0;

  this.observerServiceObserver_ = new G_ObserverServiceObserver(
                                        'xpcom-shutdown',
                                        BindToObject(this.cancel, this));

  // Ask the timer to use nsITimerCallback (.notify()) when ready
  this.timer_.initWithCallback(this, delayMS, type);
}

/**
 * Cancel this timer 
 */
G_Alarm.prototype.cancel = function() {
  if (!this.timer_) {
    return;
  }

  this.timer_.cancel();
  // Break circular reference created between this.timer_ and the G_Alarm
  // instance (this)
  this.timer_ = null;
  this.callback_ = null;

  // We don't need the shutdown observer anymore
  this.observerServiceObserver_.unregister();
}

/**
 * Invoked by the timer when it fires
 * 
 * @param timer Reference to the nsITimer which fired (not currently 
 *              passed along)
 */
G_Alarm.prototype.notify = function(timer) {
  // fire callback and save results
  var ret = this.callback_();
  
  // If they've given us a max number of times to fire, enforce it
  this.nTimes_++;
  if (this.repeating_ && 
      typeof this.maxTimes_ == "number" 
      && this.nTimes_ >= this.maxTimes_) {
    this.cancel();
  } else if (!this.repeating_) {
    // Clear out the callback closure for TYPE_ONE_SHOT timers
    this.cancel();
  }
  // We don't cancel/cleanup timers that repeat forever until either
  // xpcom-shutdown occurs or cancel() is called explicitly.

  return ret;
}

/**
 * XPCOM cruft
 */
G_Alarm.prototype.QueryInterface = function(iid) {
  if (iid.equals(Components.interfaces.nsISupports) ||
      iid.equals(Components.interfaces.nsITimerCallback))
    return this;

  throw Components.results.NS_ERROR_NO_INTERFACE;
}


/**
 * An alarm with the additional property that it cancels itself if its 
 * callback returns true.
 *
 * For parameter documentation, see G_Alarm
 */
function G_ConditionalAlarm(callback, delayMS, opt_repeating, opt_maxTimes) {
  G_Alarm.call(this, callback, delayMS, opt_repeating, opt_maxTimes);
  this.debugZone = "conditionalalarm";
}

G_ConditionalAlarm.inherits(G_Alarm);

/**
 * Invoked by the timer when it fires
 * 
 * @param timer Reference to the nsITimer which fired (not currently 
 *              passed along)
 */
G_ConditionalAlarm.prototype.notify = function(timer) {
  // Call G_Alarm::notify
  var rv = G_Alarm.prototype.notify.call(this, timer);

  if (this.repeating_ && rv) {
    G_Debug(this, "Callback of a repeating alarm returned true; cancelling.");
    this.cancel();
  }
}
