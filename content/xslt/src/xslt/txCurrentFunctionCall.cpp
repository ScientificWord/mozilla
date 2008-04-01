#include "txAtoms.h"
#include "txXSLTFunctions.h"
#include "txExecutionState.h"

/*
  Implementation of XSLT 1.0 extension function: current
*/

/**
 * Creates a new current function call
**/
CurrentFunctionCall::CurrentFunctionCall() 
{
}

/*
 * Evaluates this Expr
 *
 * @return NodeSet containing the context node used for the complete
 * Expr or Pattern.
 */
nsresult
CurrentFunctionCall::evaluate(txIEvalContext* aContext, txAExprResult** aResult)
{
    *aResult = nsnull;

    if (!requireParams(0, 0, aContext))
        return NS_ERROR_XPATH_BAD_ARGUMENT_COUNT;

    txExecutionState* es = 
        static_cast<txExecutionState*>(aContext->getPrivateContext());
    if (!es) {
        NS_ASSERTION(0,
            "called xslt extension function \"current\" with wrong context");
        return NS_ERROR_UNEXPECTED;
    }
    return aContext->recycler()->getNodeSet(
           es->getEvalContext()->getContextNode(), aResult);
}

Expr::ResultType
CurrentFunctionCall::getReturnType()
{
    return NODESET_RESULT;
}

PRBool
CurrentFunctionCall::isSensitiveTo(ContextSensitivity aContext)
{
    return !!(aContext & PRIVATE_CONTEXT);
}

#ifdef TX_TO_STRING
nsresult
CurrentFunctionCall::getNameAtom(nsIAtom** aAtom)
{
    *aAtom = txXSLTAtoms::current;
    NS_ADDREF(*aAtom);
    return NS_OK;
}
#endif
