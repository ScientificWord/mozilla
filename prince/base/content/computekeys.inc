#include productname.inc
#ifndef PROD_SW
<keyset id="msiComputeKeySet">
  <key id="MSIevalKey"       modifiers="accel" key="&MSIComputeEvalCmd.keybinding;"  oncommand="doComputeCommand('cmd_compute_Evaluate')"   /> 
	<key id="MSIdefineKey"     modifiers="accel" key="&MSIComputeDefineCmd.keybinding;" oncommand="doComputeCommand('cmd_compute_Define')"/>    
	<key id="MSIInterpretKey"  modifiers="accel shift" key="?" oncommand="doComputeCommand('cmd_compute_Interpret')"/>            
</keyset>                                                                                                         
#endif