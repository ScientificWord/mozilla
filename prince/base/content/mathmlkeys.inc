	<keyset id="msiMathMLKeySet">
	  <key id="MSIinlineMathKey" modifiers="accel" key="m" command="cmd_MSImathtext"/>
	  <key id="MSIinlineTextKey" modifiers="accel" key="t" command="cmd_MSItextmath"/>
	  <key id="MSIdisplayKey"    modifiers="accel" key="d"    command="cmd_MSIdisplayMathCmd"/>
	  <key id="MSIfractionKey"   modifiers="accel" key="/"   command="cmd_MSIfractionCmd"/>
  	<key id="MSIIntegralKey"   modifiers="accel" key="i" oncommand="insertOperator('&#x222B;', 'auto', 'auto');"/>
    <key id="MSIIntegralKey2"  modifiers="accel" key="8" oncommand="insertOperator('&#x222B;', 'auto', 'auto');"/>
    <key id="MSISummationKey"  modifiers="accel" key="7" oncommand="insertOperator('&#x03A3;', 'auto', 'auto');"/>
	  <key id="MSIradicalKey"    modifiers="accel" key="r"    command="cmd_MSIradicalCmd"/>
	  <key id="MSIAltsupKey"     modifiers="accel" key="h"        command="cmd_MSIsupCmd"/>
	  <key id="MSISupKey"        modifiers="accel" keycode="VK_UP"                   command="cmd_MSIsupCmd"/>
	  <key id="MSIAltsubKey"     modifiers="accel" key="l"        command="cmd_MSIsubCmd"/>
	  <key id="MSISubKey"        modifiers="accel" keycode="VK_DOWN"                   command="cmd_MSIsubCmd"/>
	  <key id="MSIparenKey"      modifiers="accel" key="&MSIparenCmd.keybinding;"      command="cmd_MSIparenCmd"/>
	  <key id="MSIbracketKey"    modifiers="accel" key="&MSIbracketCmd.keybinding;"    command="cmd_MSIbracketCmd"/>
	  <key id="MSIbraceKey"      modifiers="accel shift" key="&MSIbraceCmd.keybinding;" command="cmd_MSIbraceCmd"/>
	  <key id="MSIabsvalueKey"   modifiers="accel" key="&MSIabsvalueCmd.keybinding;"   command="cmd_MSIabsvalueCmd"/>
	  <key id="MSInormKey"       modifiers="accel shift" key="&MSInormCmd.keybinding;"   command="cmd_MSInormCmd"/>
	  <key id="MSIanglebracketsKey" modifiers="accel shift" key="&lt;" oncommand="insertfence('&#x2329;','&#x232A;',null)"/>
	  <key id="MSIGreekKey"      modifiers="accel" key="g"                             command="cmd_oneshotGreek"/>
	  <key id="MSISymbolKey"     modifiers="accel" key="k"                             command="cmd_oneshotSymbol"/>
	</keyset>

