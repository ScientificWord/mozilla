// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.

#ifndef msiEditorLog_h___
#define msiEditorLog_h___

#include "nsCOMPtr.h"
#include "nsHTMLEditorLog.h"
#include "msiEditor.h"

class msiEditorLog : public msiEditor, public nsHTMLEditorLog
{
public:
           msiEditorLog();
   virtual ~msiEditorLog();

  // nsISupports
  NS_DECL_ISUPPORTS_INHERITED
  
  // msiIEditor overrides
  NS_IMETHOD InsertInlineMath(void);
  NS_IMETHOD InsertDisplay(void);
  NS_IMETHOD InsertSuperscript(void);
  NS_IMETHOD InsertSubscript(void); 
  NS_IMETHOD InsertFraction(const nsAString & lineWidth, PRUint32 attrFlags); 
  NS_IMETHOD InsertBinomial(const nsAString & openingDelim, const nsAString & closingDelim, const nsAString & lineWidth, PRUint32 attrFlags); 
  NS_IMETHOD InsertSqRoot(void); 
  NS_IMETHOD InsertRoot(void); 
  NS_IMETHOD InsertSymbol(const nsAString & symbol); 
  NS_IMETHOD InsertMathname(const nsAString & name);
  NS_IMETHOD InsertEngineFunction(const nsAString & name);
  NS_IMETHOD InsertFence(const nsAString & open, const nsAString & close);
  NS_IMETHOD InsertMatrix(PRUint32 rows, PRUint32 cols, const nsAString & rowSignature,
    const nsAString & delim);
  NS_IMETHOD InsertOperator(const nsAString & symbol, PRUint32 attrFlags, const nsAString & leftspace, const nsAString & rightspace, const nsAString & minsize, const nsAString & maxsize);
  NS_IMETHOD InsertDecoration(const nsAString & above, const nsAString & below, const nsAString & aroundNotation, const nsAString & aroundType);
  // End of msiIEditor overrides
  
  
  // nsIPlaintextEditor overrides
  NS_IMETHOD InsertText(const nsAString &aStringToInsert);
  // End of nsPlaintextEditor overrides

};

#endif // msiEditorLog_h___
