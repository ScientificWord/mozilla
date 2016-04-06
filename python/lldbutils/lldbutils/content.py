import lldb
from lldbutils import utils

def summarize_text_fragment(valobj, internal_dict):
    content_union = valobj.GetChildAtIndex(0)
    state_union = valobj.GetChildAtIndex(1).GetChildMemberWithName("mState")
    length = state_union.GetChildMemberWithName("mLength").GetValueAsUnsigned(0)
    if state_union.GetChildMemberWithName("mIs2b").GetValueAsUnsigned(0):
        field = "m2b"
    else:
        field = "m1b"
    ptr = content_union.GetChildMemberWithName(field)
    return utils.format_string(ptr, length)

def ptag(debugger, command, result, dict):
    """Displays the tag name of a content node."""
    debugger.HandleCommand("expr (" + command + ")->mNodeInfo.mRawPtr->mInner.mName")

# def summarize_node(valobj, internal_dict)
#     print(valobj.GetName())
#     command = valobj.GetName()
#     print(valobj.TypeIsPointerType())
#     debugger.HandleCommand("expression nsEditor::DumpTagName(" + command + ")")
    

def modcommand(debugger, command, result, dict):
    """Adds .mRawPtr to a dom node name if necessary."""
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    frame = thread.GetSelectedFrame()
    obj = frame.EvaluateExpression(command)
    newcommand = ''
    if obj.TypeIsPointerType():
        newcommand = command
    else:
        name = obj.GetType().GetUnqualifiedType().GetName()
        if name.startswith("nsCOMPtr<") or name.startswith("nsRefPtr>") or name.startswith("nsAutoPtr<") or name.startswith("already_addRefed<"):
            newcommand = command + '.mRawPtr'
        else:
            if name.startswith("mozilla:RefPtr<"):
                newcommand = command + '.ptr'
    return newcommand


def mnode(debugger, command, result, dict):
    """Displays tag name and attributes of a content element."""
    newcommand = modcommand(debugger, command, result, dict)
    if len(newcommand) > 0:
        # print("expression " + istextnodefragment(newcommand) + "nsEditor::DumpTagName(" + newcommand + ", text),text)")
        debugger.HandleCommand("expression " + istextnodefragment(newcommand) + "(nsEditor::DumpTagName(" + newcommand + ", text),text))")
        # else:
        #     if name.startswith("mozilla:RefPtr<"):
        #         debugger.HandleCommand("expression nsEditor::DumpTagName(" + command + ".ptr)")

def istextnodefragment(newcommand):
    return 'PRUint16 nodeType; nsString text; '+ newcommand + '->GetNodeType(&nodeType); (nodeType == 3 ? (' + newcommand + '->GetNodeValue(text),text) : '

def mparent(debugger, command, result, dict):
    """Displays parent of a content element."""
    newcommand = modcommand(debugger, command, result, dict)
    if len(newcommand) > 0:
        debugger.HandleCommand("expression nsIDOMNode* parent; " + newcommand + "->GetParentNode(&parent),parent;")


def mfirstchild(debugger, command, result, dict):
    """Displays first child of a content element."""
    newcommand = modcommand(debugger, command, result, dict)
    if len(newcommand) > 0:
        debugger.HandleCommand("expression nsIDOMNode* child; " + newcommand + "->GetFirstChild(&child),child;")

def mnext(debugger, command, result, dict):
    """Displays next sibling of a content element."""
    newcommand = modcommand(debugger, command, result, dict)
    if len(newcommand) > 0:
        debugger.HandleCommand("expression nsIDOMNode* child; " + newcommand + "->GetNextSibling(&child),child;")

def mprev(debugger, command, result, dict):
    """Displays previous sibling of a content element."""
    newcommand = modcommand(debugger, command, result, dict)
    if len(newcommand) > 0:
        debugger.HandleCommand("expression nsIDOMNode* child; " + newcommand + "->GetPrevSibling(&child),child;")




def init(debugger):
    debugger.HandleCommand("type summary add nsTextFragment -F lldbutils.content.summarize_text_fragment")
    # debugger.HandleCommand("type summary add nsIDOMElement -F libdutils.content.summarize_node")
    debugger.HandleCommand("command script add -f lldbutils.content.ptag ptag")
    debugger.HandleCommand("command script add -f lldbutils.content.mnode mnode")
    debugger.HandleCommand("command script add -f lldbutils.content.mparent mparent")
    debugger.HandleCommand("command script add -f lldbutils.content.mfirstchild mfirstchild")
    debugger.HandleCommand("command script add -f lldbutils.content.mnext mnext")
    debugger.HandleCommand("command script add -f lldbutils.content.mprev mprev")
