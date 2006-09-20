// Copyright (c) 2006, MacKichan Software, Inc.  All rights reserved.
#ifndef msiRequiredArgument_h___
#define msiRequiredArgument_h___

#include "nsIEditor.h"
#include "nsIDOMNode.h"
#include "nsCOMPtr.h"
#include "nsIArray.h"

class msiRequiredArgument
{
public:
static nsresult doInsertNodes(nsIEditor * editor,
                              nsISelection * selection,
                              nsCOMPtr<nsIDOMNode> & parent, 
                              nsCOMPtr<nsIDOMNode> & requiredArg, 
                              PRBool atRight,
                              nsIArray * nodeList,
                              PRBool  deleteExisting, 
                              PRUint32 flags);

static nsresult MakeRequiredArgument(nsIEditor * editor,
                                     nsIDOMNode * leftNode, 
                                     nsIDOMNode * rightNode, 
                                     nsCOMPtr<nsIDOMNode> & argument);
static PRBool NestRequiredArgumentInMrow(nsIDOMNode * node);
                       
};                  

#endif // msiRequiredArgument_h___
