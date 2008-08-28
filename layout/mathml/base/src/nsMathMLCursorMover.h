#ifndef nsMathMLCursorMover_h__
#define nsMathMLCursorMover_h__


#include "nsCOMPtr.h"
#include "nsIMathMLCursorMover.h"
#include "nsIFrame.h"


class nsMathMLCursorMover : public nsIMathMLCursorMover
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSIMATHMLCURSORMOVER
};

#endif //nsMathMLCursorMover_h__