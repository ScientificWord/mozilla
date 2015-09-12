<stringbundleset id="stringbundleset">
  <stringbundle id="bundle_FastCursorBar">
    NotFound=Not found
  </stringbundle>
</stringbundleset>
<commandset id="baseMenuCommandSet"/>
<keyset id="baseMenuKeyset"/>

<commandset id="tasksCommands">
  <command id="cmd_unimp" oncommand="alert('Command not yet implemented');"/>
  <command id="cmd_open" oncommand="msiGoDoCommand('cmd_open')"/>
  <command id="cmd_print" oncommand="msiGoDoCommand('cmd_print')"/>
  <command id="cmd_quit" oncommand="doQuit()"/>
  <command id="cmd_save" oncommand="msiGoDoCommand('cmd_save')"/>
  <!-- and id="cmd_MSIComputeEval" oncommand="doComputeCommand2(event, 'cmd_compute_Evaluate')"/>
  <command id="cmd_MSIComputeEvalNum" oncommand="doComputeCommand('cmd_compute_EvaluateNumeric')"/> -->
</commandset>

<broadcasterset id="broadcasterset"/>
<broadcaster id="args" value=""/>
<broadcaster id="filename" value=""/>
<broadcaster id="canPrint"/>

<broadcasterset id="mainBroadcasterSet">
</broadcasterset>
<broadcasterset id="vcam_broadcasterset"/>


<keyset id="editorKeys">
  <keyset id="tasksKeys"/>
  <keyset id="MSIEditorKeys"/>
  <key id="showHideSidebar"/>
  <key id="key_newDocument" key="&filenew.keybinding;" observes="cmd_newDocument"  modifiers="accel"/>
  <key id="openeditorkb"   key="&fileopen.keybinding;"  observes="cmd_open"  modifiers="accel"/>
  <key id="savekb"         key="&filesave.keybinding;"  observes="cmd_save"  modifiers="accel"/>
  <key id="printkb"        key="&fileprint.keybinding;" observes="cmd_print" modifiers="accel"/>
  <key id="key_quit"       key="&filequit.keybinding;"  observes="cmd_quit" modifiers="accel"/>
#ifndef XP_MACOSX
  <key id="keyhelpcontents" keycode="&helpmenuContents.keybinding;" observes="cmd_help_contents"/>
#endif
#ifdef XP_MACOSX
  <key id="keyhelpcontents" keycode="/" modifiers="accel,shift" observes="cmd_help_contents"/>
#endif
</keyset>
<keyset id="mainKeyset"/> <!-- for extensions-->