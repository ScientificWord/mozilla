//	standard MSI header goes here
#ifndef ArrowStateService_h_
#define ArrowStateService_h_
 
#include "nsIDOMEvent.h"
#include "nsIObserver.h"
#include "nsString.h"
#include "prinrval.h"
#include "msiIArrowStateService.h"


class msiArrowStateService : msiIArrowStateService, nsIObserver {
public:
/** the default constructor
*/
  msiArrowStateService();

/** default destructor; not virtual since we see no possibility of 
  derivation*/
  virtual ~msiArrowStateService();
  
static  msiArrowStateService * GetInstance();
static  void ReleaseInstance();

NS_DECL_ISUPPORTS
NS_DECL_MSIIARROWSTATESERVICE
NS_DECL_NSIOBSERVER

protected:
  void ResetLastArrowData(void);
  void SetLastArrowData(PRUint32 keyCode );
  NS_IMETHOD PrefsReset(void);
  NS_IMETHOD Init(void);
  
/** The data in the state*/
  static PRUint32 minArrowCode, maxArrowCode;
  PRUint32 mCurrArrowCode; /* zero if no arrow key is considered down*/
  PRUint32 mLastArrowCodePressedAndReleased;
  PRInt32 mTapCount;
  PRInt32 mMillisecondsForTaps;
  PRIntervalTime mIntervalForTaps;
  PRIntervalTime mlastKeyPressTimes[5];
/* keep track 
  of the last N times when the arrow key in 
  mLastArrowCodePressedAndReleased was pressed.  If N presses of 
  that arrow key come within the specified interval and no other key has been 
  pressed in the interval, we start accepting automatically generated 
  key repeats. */
  PRBool misRepeating;
  PRBool misFirstArrowPress;
  PRBool misEnabled;  // When this is false, we don't do anything.
//  PRBool mdidSearch;  We don't need this unless we move the arrow key action to the KeyUp phase.
  nsString mstrFindBuffer;
  static msiArrowStateService *sInstance;
};
#endif //fastCursorArrowState_h_


