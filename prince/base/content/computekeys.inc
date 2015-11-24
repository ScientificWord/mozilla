#include productname.inc
#ifndef PROD_SW
<keyset id="msiComputeKeySet">
  <key id="MSIdefineKey"     modifiers="accel,alt" key="&MSIComputeDefineCmd.keybinding;" oncommand="doComputeCommand('cmd_compute_Define')"/>    
  <key id="MSIdefineKey2"     modifiers="accel" key="&MSIComputeDefineCmd.keybinding;" oncommand="doComputeCommand('cmd_compute_Define')"/>    
  <key id="MSIevalKey"       modifiers="accel" key="&MSIComputeEvalCmd.keybinding;"  oncommand="doComputeCommand('cmd_compute_Evaluate')" /> 
  <key id="MSIevalNumKey"    modifiers="accel,shift" key="&MSIComputeEvalNumCmd.keybinding;"  oncommand="doComputeCommand('cmd_compute_EvaluateNumeric')" /> 
  <key id="MSIInterpretKey"  modifiers="accel shift" key="?" oncommand="doComputeCommand('cmd_compute_Interpret')"/>            
</keyset>                                                                                                         
#endif