/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * ***** BEGIN LICENSE BLOCK *****
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
 * Portions created by the Initial Developer are Copyright (C) 2001
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Stuart Parmenter <pavlov@netscape.com>
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

#include "nsTimerImpl.h"
#include "TimerThread.h"
#include "nsAutoLock.h"
#include "nsAutoPtr.h"
#include "nsVoidArray.h"
#include "nsThreadManager.h"
#include "nsThreadUtils.h"
#include "prmem.h"

static PRInt32          gGenerator = 0;
static TimerThread*     gThread = nsnull;

#ifdef DEBUG_TIMERS
#include <math.h>

double nsTimerImpl::sDeltaSumSquared = 0;
double nsTimerImpl::sDeltaSum = 0;
double nsTimerImpl::sDeltaNum = 0;

static void
myNS_MeanAndStdDev(double n, double sumOfValues, double sumOfSquaredValues,
                   double *meanResult, double *stdDevResult)
{
  double mean = 0.0, var = 0.0, stdDev = 0.0;
  if (n > 0.0 && sumOfValues >= 0) {
    mean = sumOfValues / n;
    double temp = (n * sumOfSquaredValues) - (sumOfValues * sumOfValues);
    if (temp < 0.0 || n <= 1)
      var = 0.0;
    else
      var = temp / (n * (n - 1));
    // for some reason, Windows says sqrt(0.0) is "-1.#J" (?!) so do this:
    stdDev = var != 0.0 ? sqrt(var) : 0.0;
  }
  *meanResult = mean;
  *stdDevResult = stdDev;
}
#endif

NS_IMPL_THREADSAFE_QUERY_INTERFACE1(nsTimerImpl, nsITimer)
NS_IMPL_THREADSAFE_ADDREF(nsTimerImpl)

NS_IMETHODIMP_(nsrefcnt) nsTimerImpl::Release(void)
{
  nsrefcnt count;

  NS_PRECONDITION(0 != mRefCnt, "dup release");
  count = PR_AtomicDecrement((PRInt32 *)&mRefCnt);
  NS_LOG_RELEASE(this, count, "nsTimerImpl");
  if (count == 0) {
    mRefCnt = 1; /* stabilize */

    /* enable this to find non-threadsafe destructors: */
    /* NS_ASSERT_OWNINGTHREAD(nsTimerImpl); */
    NS_DELETEXPCOM(this);
    return 0;
  }

  // If only one reference remains, and mArmed is set, then the ref must be
  // from the TimerThread::mTimers array, so we Cancel this timer to remove
  // the mTimers element, and return 0 if Cancel in fact disarmed the timer.
  //
  // We use an inlined version of nsTimerImpl::Cancel here to check for the
  // NS_ERROR_NOT_AVAILABLE code returned by gThread->RemoveTimer when this
  // timer is not found in the mTimers array -- i.e., when the timer was not
  // in fact armed once we acquired TimerThread::mLock, in spite of mArmed
  // being true here.  That can happen if the armed timer is being fired by
  // TimerThread::Run as we race and test mArmed just before it is cleared by
  // the timer thread.  If the RemoveTimer call below doesn't find this timer
  // in the mTimers array, then the last ref to this timer is held manually
  // and temporarily by the TimerThread, so we should fall through to the
  // final return and return 1, not 0.
  //
  // The original version of this thread-based timer code kept weak refs from
  // TimerThread::mTimers, removing this timer's weak ref in the destructor,
  // but that leads to double-destructions in the race described above, and
  // adding mArmed doesn't help, because destructors can't be deferred, once
  // begun.  But by combining reference-counting and a specialized Release
  // method with "is this timer still in the mTimers array once we acquire
  // the TimerThread's lock" testing, we defer destruction until we're sure
  // that only one thread has its hot little hands on this timer.
  //
  // Note that both approaches preclude a timer creator, and everyone else
  // except the TimerThread who might have a strong ref, from dropping all
  // their strong refs without implicitly canceling the timer.  Timers need
  // non-mTimers-element strong refs to stay alive.

  if (count == 1 && mArmed) {
    mCanceled = PR_TRUE;

    NS_ASSERTION(gThread, "An armed timer exists after the thread timer stopped.");
    if (NS_SUCCEEDED(gThread->RemoveTimer(this)))
      return 0;
  }

  return count;
}

nsTimerImpl::nsTimerImpl() :
  mClosure(nsnull),
  mCallbackType(CALLBACK_TYPE_UNKNOWN),
  mFiring(PR_FALSE),
  mArmed(PR_FALSE),
  mCanceled(PR_FALSE),
  mGeneration(0),
  mDelay(0),
  mTimeout(0)
{
  // XXXbsmedberg: shouldn't this be in Init()?
  mCallingThread = do_GetCurrentThread();

  mCallback.c = nsnull;

#ifdef DEBUG_TIMERS
  mStart = 0;
  mStart2 = 0;
#endif
}

nsTimerImpl::~nsTimerImpl()
{
  ReleaseCallback();
}

//static
nsresult
nsTimerImpl::Startup()
{
  nsresult rv;

  gThread = new TimerThread();
  if (!gThread) return NS_ERROR_OUT_OF_MEMORY;

  NS_ADDREF(gThread);
  rv = gThread->InitLocks();

  if (NS_FAILED(rv)) {
    NS_RELEASE(gThread);
  }

  return rv;
}

void nsTimerImpl::Shutdown()
{
#ifdef DEBUG_TIMERS
  if (PR_LOG_TEST(gTimerLog, PR_LOG_DEBUG)) {
    double mean = 0, stddev = 0;
    myNS_MeanAndStdDev(sDeltaNum, sDeltaSum, sDeltaSumSquared, &mean, &stddev);

    PR_LOG(gTimerLog, PR_LOG_DEBUG, ("sDeltaNum = %f, sDeltaSum = %f, sDeltaSumSquared = %f\n", sDeltaNum, sDeltaSum, sDeltaSumSquared));
    PR_LOG(gTimerLog, PR_LOG_DEBUG, ("mean: %fms, stddev: %fms\n", mean, stddev));
  }
#endif

  if (!gThread)
    return;

  gThread->Shutdown();
  NS_RELEASE(gThread);
}


nsresult nsTimerImpl::InitCommon(PRUint32 aType, PRUint32 aDelay)
{
  nsresult rv;

  NS_ENSURE_TRUE(gThread, NS_ERROR_NOT_INITIALIZED);

  rv = gThread->Init();
  NS_ENSURE_SUCCESS(rv, rv);

  /**
   * In case of re-Init, both with and without a preceding Cancel, clear the
   * mCanceled flag and assign a new mGeneration.  But first, remove any armed
   * timer from the timer thread's list.
   *
   * If we are racing with the timer thread to remove this timer and we lose,
   * the RemoveTimer call made here will fail to find this timer in the timer
   * thread's list, and will return false harmlessly.  We test mArmed here to
   * avoid the small overhead in RemoveTimer of locking the timer thread and
   * checking its list for this timer.  It's safe to test mArmed even though
   * it might be cleared on another thread in the next cycle (or even already
   * be cleared by another CPU whose store hasn't reached our CPU's cache),
   * because RemoveTimer is idempotent.
   */
  if (mArmed)
    gThread->RemoveTimer(this);
  mCanceled = PR_FALSE;
  mGeneration = PR_AtomicIncrement(&gGenerator);

  mType = (PRUint8)aType;
  SetDelayInternal(aDelay);

  return gThread->AddTimer(this);
}

NS_IMETHODIMP nsTimerImpl::InitWithFuncCallback(nsTimerCallbackFunc aFunc,
                                                void *aClosure,
                                                PRUint32 aDelay,
                                                PRUint32 aType)
{
  NS_ENSURE_ARG_POINTER(aFunc);
  
  ReleaseCallback();
  mCallbackType = CALLBACK_TYPE_FUNC;
  mCallback.c = aFunc;
  mClosure = aClosure;

  return InitCommon(aType, aDelay);
}

NS_IMETHODIMP nsTimerImpl::InitWithCallback(nsITimerCallback *aCallback,
                                            PRUint32 aDelay,
                                            PRUint32 aType)
{
  NS_ENSURE_ARG_POINTER(aCallback);

  ReleaseCallback();
  mCallbackType = CALLBACK_TYPE_INTERFACE;
  mCallback.i = aCallback;
  NS_ADDREF(mCallback.i);

  return InitCommon(aType, aDelay);
}

NS_IMETHODIMP nsTimerImpl::Init(nsIObserver *aObserver,
                                PRUint32 aDelay,
                                PRUint32 aType)
{
  NS_ENSURE_ARG_POINTER(aObserver);

  ReleaseCallback();
  mCallbackType = CALLBACK_TYPE_OBSERVER;
  mCallback.o = aObserver;
  NS_ADDREF(mCallback.o);

  return InitCommon(aType, aDelay);
}

NS_IMETHODIMP nsTimerImpl::Cancel()
{
  mCanceled = PR_TRUE;

  if (gThread)
    gThread->RemoveTimer(this);

  ReleaseCallback();

  return NS_OK;
}

NS_IMETHODIMP nsTimerImpl::SetDelay(PRUint32 aDelay)
{
  // If we're already repeating precisely, update mTimeout now so that the
  // new delay takes effect in the future.
  if (mTimeout != 0 && mType == TYPE_REPEATING_PRECISE)
    mTimeout = PR_IntervalNow();

  SetDelayInternal(aDelay);

  if (!mFiring && gThread)
    gThread->TimerDelayChanged(this);

  return NS_OK;
}

NS_IMETHODIMP nsTimerImpl::GetDelay(PRUint32* aDelay)
{
  *aDelay = mDelay;
  return NS_OK;
}

NS_IMETHODIMP nsTimerImpl::SetType(PRUint32 aType)
{
  mType = (PRUint8)aType;
  // XXX if this is called, we should change the actual type.. this could effect
  // repeating timers.  we need to ensure in Fire() that if mType has changed
  // during the callback that we don't end up with the timer in the queue twice.
  return NS_OK;
}

NS_IMETHODIMP nsTimerImpl::GetType(PRUint32* aType)
{
  *aType = mType;
  return NS_OK;
}


NS_IMETHODIMP nsTimerImpl::GetClosure(void** aClosure)
{
  *aClosure = mClosure;
  return NS_OK;
}


NS_IMETHODIMP nsTimerImpl::GetCallback(nsITimerCallback **aCallback)
{
  if (mCallbackType == CALLBACK_TYPE_INTERFACE)
    NS_IF_ADDREF(*aCallback = mCallback.i);
  else if (mTimerCallbackWhileFiring)
    NS_ADDREF(*aCallback = mTimerCallbackWhileFiring);
  else
    *aCallback = nsnull;

  return NS_OK;
}


void nsTimerImpl::Fire()
{
  if (mCanceled)
    return;

  PRIntervalTime now = PR_IntervalNow();
#ifdef DEBUG_TIMERS
  if (PR_LOG_TEST(gTimerLog, PR_LOG_DEBUG)) {
    PRIntervalTime a = now - mStart; // actual delay in intervals
    PRUint32       b = PR_MillisecondsToInterval(mDelay); // expected delay in intervals
    PRUint32       d = PR_IntervalToMilliseconds((a > b) ? a - b : b - a); // delta in ms
    sDeltaSum += d;
    sDeltaSumSquared += double(d) * double(d);
    sDeltaNum++;

    PR_LOG(gTimerLog, PR_LOG_DEBUG, ("[this=%p] expected delay time %4dms\n", this, mDelay));
    PR_LOG(gTimerLog, PR_LOG_DEBUG, ("[this=%p] actual delay time   %4dms\n", this, PR_IntervalToMilliseconds(a)));
    PR_LOG(gTimerLog, PR_LOG_DEBUG, ("[this=%p] (mType is %d)       -------\n", this, mType));
    PR_LOG(gTimerLog, PR_LOG_DEBUG, ("[this=%p]     delta           %4dms\n", this, (a > b) ? (PRInt32)d : -(PRInt32)d));

    mStart = mStart2;
    mStart2 = 0;
  }
#endif

  PRIntervalTime timeout = mTimeout;
  if (mType == TYPE_REPEATING_PRECISE) {
    // Precise repeating timers advance mTimeout by mDelay without fail before
    // calling Fire().
    timeout -= PR_MillisecondsToInterval(mDelay);
  }
  if (gThread)
    gThread->UpdateFilter(mDelay, timeout, now);

  if (mCallbackType == CALLBACK_TYPE_INTERFACE)
    mTimerCallbackWhileFiring = mCallback.i;
  mFiring = PR_TRUE;
  
  // Handle callbacks that re-init the timer, but avoid leaking.
  // See bug 330128.
  CallbackUnion callback = mCallback;
  PRUintn callbackType = mCallbackType;
  if (callbackType == CALLBACK_TYPE_INTERFACE)
    NS_ADDREF(callback.i);
  else if (callbackType == CALLBACK_TYPE_OBSERVER)
    NS_ADDREF(callback.o);
  ReleaseCallback();

  switch (callbackType) {
    case CALLBACK_TYPE_FUNC:
      callback.c(this, mClosure);
      break;
    case CALLBACK_TYPE_INTERFACE:
      callback.i->Notify(this);
      break;
    case CALLBACK_TYPE_OBSERVER:
      callback.o->Observe(static_cast<nsITimer*>(this),
                          NS_TIMER_CALLBACK_TOPIC,
                          nsnull);
      break;
    default:;
  }

  // If the callback didn't re-init the timer, and it's not a one-shot timer,
  // restore the callback state.
  if (mCallbackType == CALLBACK_TYPE_UNKNOWN &&
      mType != TYPE_ONE_SHOT && !mCanceled) {
    mCallback = callback;
    mCallbackType = callbackType;
  } else {
    // The timer was a one-shot, or the callback was reinitialized.
    if (callbackType == CALLBACK_TYPE_INTERFACE)
      NS_RELEASE(callback.i);
    else if (callbackType == CALLBACK_TYPE_OBSERVER)
      NS_RELEASE(callback.o);
  }

  mFiring = PR_FALSE;
  mTimerCallbackWhileFiring = nsnull;

#ifdef DEBUG_TIMERS
  if (PR_LOG_TEST(gTimerLog, PR_LOG_DEBUG)) {
    PR_LOG(gTimerLog, PR_LOG_DEBUG,
           ("[this=%p] Took %dms to fire timer callback\n",
            this, PR_IntervalToMilliseconds(PR_IntervalNow() - now)));
  }
#endif

  if (mType == TYPE_REPEATING_SLACK) {
    SetDelayInternal(mDelay); // force mTimeout to be recomputed.
    if (gThread)
      gThread->AddTimer(this);
  }
}


class nsTimerEvent : public nsRunnable {
public:
  NS_IMETHOD Run();

  nsTimerEvent(nsTimerImpl *timer, PRInt32 generation)
    : mTimer(timer), mGeneration(generation) {
    // timer is already addref'd for us
  }

#ifdef DEBUG_TIMERS
  PRIntervalTime mInitTime;
#endif

private:
  ~nsTimerEvent() { 
#ifdef DEBUG
    if (mTimer)
      NS_WARNING("leaking reference to nsTimerImpl");
#endif
  }

  nsTimerImpl *mTimer;
  PRInt32      mGeneration;
};

NS_IMETHODIMP nsTimerEvent::Run()
{
  nsRefPtr<nsTimerImpl> timer;
  timer.swap(mTimer);

  if (mGeneration != timer->GetGeneration())
    return NS_OK;

#ifdef DEBUG_TIMERS
  if (PR_LOG_TEST(gTimerLog, PR_LOG_DEBUG)) {
    PRIntervalTime now = PR_IntervalNow();
    PR_LOG(gTimerLog, PR_LOG_DEBUG,
           ("[this=%p] time between PostTimerEvent() and Fire(): %dms\n",
            this, PR_IntervalToMilliseconds(now - mInitTime)));
  }
#endif

  timer->Fire();

  return NS_OK;
}

void nsTimerImpl::PostTimerEvent()
{
  // XXX we may want to reuse this nsTimerEvent in the case of repeating timers.

  // Since TimerThread addref'd 'this' for us, we don't need to addref here.
  // We will release in destroyMyEvent.  We need to copy the generation number
  // from this timer into the event, so we can avoid firing a timer that was
  // re-initialized after being canceled.

  nsTimerEvent* event = new nsTimerEvent(this, mGeneration);
  if (!event)
    return;

#ifdef DEBUG_TIMERS
  if (PR_LOG_TEST(gTimerLog, PR_LOG_DEBUG)) {
    event->mInitTime = PR_IntervalNow();
  }
#endif

  // If this is a repeating precise timer, we need to calculate the time for
  // the next timer to fire before we make the callback.
  if (mType == TYPE_REPEATING_PRECISE) {
    SetDelayInternal(mDelay);
    if (gThread)
      gThread->AddTimer(this);
  }

  mCallingThread->Dispatch(event, NS_DISPATCH_NORMAL);
}

void nsTimerImpl::SetDelayInternal(PRUint32 aDelay)
{
  PRIntervalTime delayInterval = PR_MillisecondsToInterval(aDelay);
  if (delayInterval > DELAY_INTERVAL_MAX) {
    delayInterval = DELAY_INTERVAL_MAX;
    aDelay = PR_IntervalToMilliseconds(delayInterval);
  }

  mDelay = aDelay;

  PRIntervalTime now = PR_IntervalNow();
  if (mTimeout == 0 || mType != TYPE_REPEATING_PRECISE)
    mTimeout = now;

  mTimeout += delayInterval;

#ifdef DEBUG_TIMERS
  if (PR_LOG_TEST(gTimerLog, PR_LOG_DEBUG)) {
    if (mStart == 0)
      mStart = now;
    else
      mStart2 = now;
  }
#endif
}

// NOT FOR PUBLIC CONSUMPTION!
nsresult
NS_NewTimer(nsITimer* *aResult, nsTimerCallbackFunc aCallback, void *aClosure,
            PRUint32 aDelay, PRUint32 aType)
{
    nsTimerImpl* timer = new nsTimerImpl();
    if (timer == nsnull)
        return NS_ERROR_OUT_OF_MEMORY;
    NS_ADDREF(timer);

    nsresult rv = timer->InitWithFuncCallback(aCallback, aClosure, 
                                              aDelay, aType);
    if (NS_FAILED(rv)) {
        NS_RELEASE(timer);
        return rv;
    }

    *aResult = timer;
    return NS_OK;
}
