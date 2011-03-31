
#include "nsIDOMNode.h"
#include "nsISelection.h"
#include "nsIRange.h"


void DumpNode(nsIDOMNode *aNode, PRInt32 indent, bool recurse /* = false */);

void DumpSelection( nsISelection * sel);

void DumpRange( nsIRange * range);

