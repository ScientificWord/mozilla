#include productname.inc
#ifndef PROD_SW
<keyset id="msiComputeKeySet">
  <key id="MSIevalKey"   modifiers="accel" key="&MSIComputeEvalCmd.keybinding;"  oncommand="doComputeCommand('cmd_compute_Evaluate')"   /> 
	<key id="MSIdefineKey"     modifiers="accel" key="=" observes="cmd_MSIComputeDefine"/>    
	<key id="MSIInterpretKey"  modifiers="accel shift" key="?" observes="cmd_MSIComputeInterpret"/>            
</keyset>                                                                                                         
#endif