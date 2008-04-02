//	standard MSI header goes here
#include "msiArrowStateService.h"
#include "nsCOMPtr.h"
#include <string.h>
#include "nsIDOMEvent.h"
#include "nsIDOMKeyEvent.h"
#include "nsIDOMNSUIEvent.h"
#include "nsServiceManagerUtils.h"
#include "nsIPrefBranch.h"
#include "nsIPrefBranchInternal.h"
#include "nsIPrefService.h"


PRUint32 msiArrowStateService::minArrowCode = (PRUint32)nsIDOMKeyEvent::DOM_VK_LEFT;
PRUint32 msiArrowStateService::maxArrowCode = (PRUint32)nsIDOMKeyEvent::DOM_VK_DOWN;

NS_INTERFACE_MAP_BEGIN(msiArrowStateService)
  NS_INTERFACE_MAP_ENTRY(msiIArrowStateService)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, msiIArrowStateService)
  NS_INTERFACE_MAP_ENTRY(nsIObserver)
NS_INTERFACE_MAP_END

NS_IMPL_ADDREF(msiArrowStateService)
NS_IMPL_RELEASE(msiArrowStateService)

msiArrowStateService* msiArrowStateService::sInstance = nsnull;



msiArrowStateService::msiArrowStateService() :
	mLastArrowCodePressedAndReleased(0), mCurrArrowCode(0), 
  mTapCount(1), mMillisecondsForTaps(500), misRepeating(PR_FALSE),
  misFirstArrowPress(PR_FALSE), misEnabled(PR_TRUE)
{
  ResetLastArrowData();
  // Notice that GetInstance also does some initialization. 
}

msiArrowStateService::~msiArrowStateService()
{
}

msiArrowStateService*
msiArrowStateService::GetInstance()
{
  if (!sInstance) {
    sInstance = new msiArrowStateService();
    if (!sInstance)
      return nsnull;

    NS_ADDREF(sInstance);  // addref for sInstance global

    if (NS_FAILED(sInstance->Reset())) {
      NS_RELEASE(sInstance);
      return nsnull;
    }
    if (NS_FAILED(sInstance->Init())) {
      NS_RELEASE(sInstance);
      return nsnull;
    }
  }
  NS_ADDREF(sInstance);   // addref for the getter

  return sInstance;
}

void 
msiArrowStateService::ReleaseInstance()
{
  nsCOMPtr<nsIPrefBranchInternal> prefInternal(do_GetService(NS_PREFSERVICE_CONTRACTID));
  if (prefInternal) {
    prefInternal->RemoveObserver("fastcursor.enabled", sInstance);
    prefInternal->RemoveObserver("fastcursor.tapsforrepeat", sInstance);
    prefInternal->RemoveObserver("fastcursor.millisecondsfortaps", sInstance);
  }
  NS_IF_RELEASE(sInstance);
}


NS_IMETHODIMP
msiArrowStateService::PrefsReset()
{
  nsCOMPtr<nsIPrefBranch> prefBranch(do_GetService(NS_PREFSERVICE_CONTRACTID));
  NS_ENSURE_TRUE(prefBranch, NS_ERROR_FAILURE); 
  nsresult rv = prefBranch->GetBoolPref("fastcursor.enabled",
                          &misEnabled);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = prefBranch->GetIntPref("fastcursor.tapsforrepeat",
                          &mTapCount);
  NS_ENSURE_SUCCESS(rv, rv);

  rv = prefBranch->GetIntPref("fastcursor.millisecondsfortaps",
                          &mMillisecondsForTaps);
  NS_ENSURE_SUCCESS(rv, rv);
  NS_ASSERTION(mTapCount <= 4, "mTapCount too big!");
  if (mTapCount > 4) mTapCount = 4;
  mIntervalForTaps = PR_MillisecondsToInterval(mMillisecondsForTaps);
}

NS_IMETHODIMP
msiArrowStateService::Init()
{
  nsCOMPtr<nsIPrefBranchInternal> prefInternal(do_GetService(NS_PREFSERVICE_CONTRACTID));
  nsresult rv = prefInternal->AddObserver("fastcursor.enabled", this, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = prefInternal->AddObserver("fastcursor.tapsforrepeat", this, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);
  rv = prefInternal->AddObserver("fastcursor.millisecondsfortaps", this, PR_FALSE);
  NS_ENSURE_SUCCESS(rv, rv);
  // ----------- Get initial preferences ----------
  PrefsReset();
}

NS_IMETHODIMP
msiArrowStateService::Observe(nsISupports *aSubject, const char *aTopic,
                         const PRUnichar *aData)
{
  if (!strcmp(aTopic, NS_PREFBRANCH_PREFCHANGE_TOPIC_ID))
    return PrefsReset();

  return NS_OK;
}


void 
msiArrowStateService::ResetLastArrowData(void)
{
  mLastArrowCodePressedAndReleased = 0;
  for (int i=0; i<5; i++) mlastKeyPressTimes[i] = (PRIntervalTime)0;
  misRepeating = PR_FALSE;
  misFirstArrowPress = PR_FALSE;
}

void 
msiArrowStateService::SetLastArrowData(PRUint32 keyCode )
{
  if (mLastArrowCodePressedAndReleased == keyCode) {
    for (int i=4; i>0; i--) mlastKeyPressTimes[i] = mlastKeyPressTimes[i-1];
  }
  else {
    if (mLastArrowCodePressedAndReleased != 0) ResetLastArrowData();
  }
  misFirstArrowPress = PR_TRUE;
  mLastArrowCodePressedAndReleased = keyCode;
  mlastKeyPressTimes[0] = PR_IntervalNow();
}


NS_IMETHODIMP 
msiArrowStateService::FindKeyCode( nsIDOMEvent* aEvent, 
		PRUint32* keyCode, PRBool* isArrow )
	/** This function does error handling, checking for PreventDefault, finding the keyCode, and also checks to see if the
  	keyCode belongs to an arrow key.  If this function returns 0 for the keyCode, do nothing. */
{
  nsCOMPtr<nsIDOMNSUIEvent> nsUIEvent = do_QueryInterface(aEvent);
  *keyCode = 0;
  *isArrow = PR_FALSE;
  if (nsUIEvent) 
  {
    PRBool defaultPrevented;
    nsUIEvent->GetPreventDefault(&defaultPrevented);
    if (defaultPrevented)
    {
      return NS_OK;
    }
  }

  nsCOMPtr<nsIDOMKeyEvent>keyEvent = do_QueryInterface(aEvent);
  if (!keyEvent) 
  {
    //non-key event passed to a key event handler, perhaps assert?
    return NS_OK;
  }
  keyEvent->GetKeyCode(keyCode);
	*isArrow = (*keyCode <= maxArrowCode) && (*keyCode >= minArrowCode);
	return NS_OK;
}

NS_IMETHODIMP 
msiArrowStateService::FindArrowKeyState( PRBool * isArrowActive, PRUint32* keyCode,
    PRBool* isAcceptingAutoRepeats, PRBool *isVertical, PRBool *isForward,
    PRUint32 inkeyCode, PRBool * isFirstArrowPress )
  /** Determines if an arrow key is down (virtually), and if one is, returns
    the keyCode of the arrow key that is down.  This may not coincide exactly 
    with the physical key being down, since it is possible to chord one arrow 
    key with another, in which case on the second arrow key is considered 
    'down'.  The key may be down physically without being considered 'down', 
    e.g., if the user has pressed a second arrow before releasing the first. See the implementation for 
    the details.  Generally autorepeats of the arrow keys are ignored
	  but be may counted if the user has pressed the arrow key 2 or 3 times in 
	  rapid succession. (See the implementation.)  The booleans isVertical 
    and isForward are for convenience.  
	  They are set according to the following table: 
						arrow key down  isVertical isForward hex   !(hex & 1)     ((hex-1)&2) 
						  left				     no		      no      25       0               0      
						  up			         yes		    no      26       1               0      
						  right				     no		      yes     27       0               1      
						  down				     yes		    yes     28       1               1       
    Since we want to suppress normal arrow action when the arrow key is pressed, EXCEPT for
    the first keypress event and those after the user has pressed the key quickly N times, we
    return isFirstArrowPress which returns true if inkeyCode is the same as the returned keyCode 
    and if no other calls of FindArrowKeyState have been made since the arrow key was pressed. The
    isAcceptingAutoRepeats is true if the key has been pressed the requisite number of times in the
    specified time period. */
{
	if (!misEnabled) {
    *isArrowActive = PR_FALSE;
    return NS_OK;
   }
  
  *keyCode = mCurrArrowCode;
  *isAcceptingAutoRepeats = misRepeating; 
	*isArrowActive = (mCurrArrowCode != 0);
  // see the table
  if (*isArrowActive) {
    *isVertical = !(mCurrArrowCode & 1);
    *isForward = ((mCurrArrowCode-1)&2);
  } else {
    *isVertical = PR_FALSE;
    *isForward = PR_FALSE;
  }
  *isFirstArrowPress = (misFirstArrowPress && (*keyCode == inkeyCode));
  misFirstArrowPress = PR_FALSE; // the variable is used only the first time 
  if (misRepeating) *isArrowActive = (*keyCode != inkeyCode);
    // if we are letting arrow keys autorepeat, then we
    // want to let the usual machinery process the arrow key presses
	return NS_OK;
}

NS_IMETHODIMP
msiArrowStateService::ArrowKeyDown( PRUint32 keyCode)
{
	if (keyCode == mCurrArrowCode)	// this must be an automatically generated keydown
		return NS_OK;
	NS_ASSERTION((keyCode <= maxArrowCode) && (keyCode >= minArrowCode),
     "Bad key code sent to msiArrowStateService::ArrowKeyDown");
	mCurrArrowCode = keyCode;
  SetLastArrowData( keyCode );
 
  // check to see if we should turn on autorepeating of arrow keys
  
  if ((mTapCount <= 0) || 
    (mlastKeyPressTimes[mTapCount] + mIntervalForTaps >= PR_IntervalNow())) 
      misRepeating = PR_TRUE;
	return NS_OK;	//consume the key press
}

NS_IMETHODIMP 
msiArrowStateService::ArrowKeyUp( PRUint32 keyCode)
{
  // if the keyCode matches the 'officially down' arrow key, say the key is now up
  // if arrow key A is pressed and then arrow key B is pressed, then B is 'officially'
  // down, and if A is then released, we do nothing.  If B is released we say none are down.
	if (keyCode == mCurrArrowCode) 
  {
    mCurrArrowCode = 0;
	  misRepeating = PR_FALSE;
  	mstrFindBuffer.Truncate(0);
  }
	return NS_OK;
}

NS_IMETHODIMP 
msiArrowStateService::NonArrowKeyPress( PRUint32 keyCode, PRUint32 charCode)
	/** Note that a non-arrow printable character has been pressed 	**/
{
  if (mCurrArrowCode != 0)  // an arrow key is down
  {
    if (keyCode == nsIDOMKeyEvent::DOM_VK_ESCAPE)
      Reset();
    else if (keyCode == nsIDOMKeyEvent::DOM_VK_BACK_SPACE) {
      if (mstrFindBuffer.Length() > 0) mstrFindBuffer.SetLength(mstrFindBuffer.Length()-1); 
    } else {
      if (mCurrArrowCode & 1) mstrFindBuffer.Truncate(0);
      mstrFindBuffer.Append( charCode );
    }
  }
	return NS_OK;
}


NS_IMETHODIMP 
msiArrowStateService::WasKeySearched( PRBool * wasSearched )
// This function is currently unused, and probably will be removed.
{
  *wasSearched = PR_FALSE;
	return NS_OK;
}

NS_IMETHODIMP 
msiArrowStateService::Reset( void )
{
	mCurrArrowCode = 0;
  mLastArrowCodePressedAndReleased = 0;
  misRepeating = PR_FALSE;
  mstrFindBuffer.Truncate(0);
  return NS_OK;
}

/* attribute AString findBuffer; */
NS_IMETHODIMP
msiArrowStateService::GetFindBuffer(nsAString & aFindBuffer)
{
    aFindBuffer = mstrFindBuffer;
    return NS_OK;
}

NS_IMETHODIMP msiArrowStateService::SetFindBuffer(const nsAString & aFindBuffer)
{
    mstrFindBuffer = aFindBuffer;
    return NS_OK;
}
