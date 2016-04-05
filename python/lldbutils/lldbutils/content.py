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
    

def mtag(debugger, command, result, dict):
    """Displays tag name and attributes of a content element."""
    target = debugger.GetSelectedTarget()
    process = target.GetProcess()
    thread = process.GetSelectedThread()
    frame = thread.GetSelectedFrame()
    obj = frame.EvaluateExpression(command)
    if obj.TypeIsPointerType():
        debugger.HandleCommand("expression nsEditor::DumpTagName(" + command + ")")
    else:
        name = obj.GetType().GetUnqualifiedType().GetName()
        if name.startswith("nsCOMPtr<") or name.startswith("nsRefPtr>") or name.startswith("nsAutoPtr<") or name.startswith("already_addRefed<"):
            debugger.HandleCommand("expression nsEditor::DumpTagName(" + command + ".mRawPtr)")
        # else:
        #     if name.startswith("mozilla:RefPtr<"):
        #         debugger.HandleCommand("expression nsEditor::DumpTagName(" + command + ".ptr)")


def init(debugger):
    debugger.HandleCommand("type summary add nsTextFragment -F lldbutils.content.summarize_text_fragment")
    # debugger.HandleCommand("type summary add nsIDOMElement -F libdutils.content.summarize_node")
    debugger.HandleCommand("command script add -f lldbutils.content.ptag ptag")
    debugger.HandleCommand("command script add -f lldbutils.content.mtag mtag")
