	<keyset id="msiMathMLKeySet">
	  <key id="MSIinlineMathKey" 		modifiers="accel" 							key="m" oncommand="msiGoDoCommand('cmd_MSIMath')"/>
	  <key id="MSIinlineTextKey" 		modifiers="accel" 							key="t" oncommand="msiGoDoCommand('cmd_MSIText')"/>
	  <key id="MSIdisplayKey"    		modifiers="accel" 							key="d" command="cmd_MSIdisplayMathCmd"/>
	  <key id="MSIfractionKey"   		modifiers="shift any accel" 		key="/" oncommand="msiGoDoCommand('cmd_MSIfractionCmd')"/>
	  <key id="MSIIntegralKey"   		modifiers="accel" 							key="i" oncommand="insertOperator('&#x222B;', 'auto', 'auto');"/>
    <key id="MSIIntegralKey2"  		modifiers="accel" 							key="8" oncommand="insertOperator('&#x222B;', 'auto', 'auto');"/>
    <key id="MSISummationKey"  		modifiers="accel" 							key="7" oncommand="insertOperator('&#x03A3;', 'auto', 'auto');"/>
	  <key id="MSIradicalKey"    		modifiers="accel" 							key="r" command="cmd_MSIradicalCmd"/>
	  <key id="MSIAltsupKey"     		modifiers="accel" 							key="h" command="cmd_MSIsupCmd"/>
	  <key id="MSISupKey"        		modifiers="accel" 							keycode="VK_UP" command="cmd_MSIsupCmd"/>
	  <key id="MSIAltsubKey"     		modifiers="accel" 							key="l"        	command="cmd_MSIsubCmd"/>
	  <key id="MSISubKey"        		modifiers="accel" 							keycode="VK_DOWN"   								command="cmd_MSIsubCmd"/>
	  <key id="MSIparenKey9"     		modifiers="accel" 							key="9" command="cmd_MSIparenCmd"/>
	  <key id="MSIparenKey0"     		modifiers="accel" 							key="0" command="cmd_MSIparenCmd"/>
	  <key id="MSIparenKey("     		modifiers="shift any accel" 		key="(" command="cmd_MSIparenCmd"/>
	  <key id="MSIparenKey)"     		modifiers="shift any accel" 		key=")" command="cmd_MSIparenCmd"/>
	  <key id="MSIbracketKey"    		modifiers="shift any accel" 		key="&MSIbracketCmd.keybinding;"    command="cmd_MSIbracketCmd"/>
	  <key id="MSIbraceKey"      		modifiers="shift any accel" 		key="&MSIbraceCmd.keybinding;" 			command="cmd_MSIbraceCmd"/>
	  <key id="MSIabsvalueKey"   		modifiers="shift any accel" 		key="&MSIabsvalueCmd.keybinding;"   command="cmd_MSIabsvalueCmd"/>
	  <key id="MSInormKey"       		modifiers="shift any accel" 		key="&MSInormCmd.keybinding;"  			command="cmd_MSInormCmd"/>
	  <key id="MSIanglebracketsKey" modifiers="shift any accel" 		key="&lt;" 			command="cmd_MSIanglebracketCmd"/>
	  <key id="MSIGreekKey"      		modifiers="accel" 							key="g"  command="cmd_oneshotGreek"/>
	  <key id="MSISymbolKey"     		modifiers="accel" 							key="k"  command="cmd_oneshotSymbol"/>
	  <key id="msiquickbacktick" 		modifiers="shift alt any accel" key="`"  oncommand="msiRevCharQuick('&#x0300;');"/>
    <key id="msiquicksquote" 			modifiers="shift alt any accel" key="'"  oncommand="msiRevCharQuick('&#x0301;');"/>
    <key id="msiquickcaret"				modifiers="shift alt any accel" key="^"  oncommand="msiRevCharQuick('&#x0302;');"/>
    <key id="msiquicktilde" 			modifiers="shift alt any accel" key="~"  oncommand="msiRevCharQuick('&#x0303;');"/>
    <key id="msiquickunderscore"		modifiers="shift alt any accel" key="_"  oncommand="msiRevCharQuick('&#x0305;');"/>
    <key id="msiquickperiod" 			modifiers="shift alt any accel" key="."  oncommand="msiRevCharQuick('&#x0307;');"/>
    <key id="msiquickdquote" 			modifiers="shift alt any accel" key='"'  oncommand="msiRevCharQuick('&#x0308;');"/>
    <key id="msiquickexclam" 			modifiers="shift alt any accel" key='!'  oncommand="msiRevCharQuick('&#x20d7;');"/>
    <key id="msiquickvector" 			modifiers="shift alt any accel" key='-'  oncommand="msiRevCharQuick('&#x20d7;');"/>
	</keyset>

