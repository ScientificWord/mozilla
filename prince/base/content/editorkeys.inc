<keyset id="msiEditorKeys">
  <key id="msiAltRedoKey" key="y" command="cmd_redo" modifiers="accel" />
  <key id="msiArrowDownKey" keycode="VK_DOWN" command="cmd_lineNext"/>
  <key id="msiArrowNextKey" keycode="VK_RIGHT" command="cmd_charNext"/>
  <key id="msiArrowPrevKey" keycode="VK_LEFT" command="cmd_charPrevious"/>
  <key id="msiArrowUpKey" keycode="VK_UP" command="cmd_linePrevious"/>
  <key id="msiBoldKey" modifiers="accel" key="b" oncommand="msiDoStatefulCommand('cmd_texttag','bold');"/>    
  <key id="msiCloseFileKey" modifiers="accel" key="w" observes="cmd_close"/>   
  <key id="msiCloseFileKeyWin" modifiers="accel" keycode="VK_F4" observes="cmd_close"/>
  <key id="msiCopyKey" key="c" command="cmd_copy" modifiers="accel"/>
  <key id="msiCutKey" key="x" command="cmd_cut" modifiers="accel"/>
  <key id="msiDeleteKey" keycode="VK_DELETE" command="cmd_delete" modifiers="accel"/>
  <key id="msiFindKey" key="f" command="cmd_find" modifiers="accel"/>
  <key id="msiFindNextKey" keycode="VK_F3" keytext="F3" command="cmd_findNext" />
  <key id="msiFindPrevKey" keycode="VK_F3" command="cmd_findPrev" modifiers="shift"/>
  <key id="msiFMTagKey" key="4" action="document.getElementById('OtherTagSelections').focus();" modifiers="accel"/>
  <key id="msiGreekKeyboardKey" key="g" modifiers="accel"   command="cmd_oneshotGreek"/>
  <key id="msiInsertSubstructKey" key=">" command="cmd_insertSubstructure" modifiers="accel,shift"/>
  <key id="msiMacHelp" key="?" modifiers="accel,shift" command="cmd_showhelp"/>
  <key id="msiNewFileKey" key="n" modifiers="accel" oncommand="openNewDocument();"/>
  <key id="msiOpenFileKey" key="o" modifiers="accel" oncommand = "openDocument();" />
  <key id="msiParaTagKey" key="2" action="document.getElementById('ParaTagSelections').focus();" modifiers="accel"/>
  <key id="msiPasteKey" key="v" command="cmd_paste" modifiers="accel"/>
  <key id="msiPasteKeyNoFormatting" key="v" command="cmd_pasteNoFormatting" modifiers="accel,shift"/>
  <key id="msiRedoKey" key="z" command="cmd_redo" modifiers="accel,shift" />
  <key id="msiSaveAsKey" key="s" command="cmd_saveAs" modifiers="accel,shift" />
  <key id="msiSelectAllKey" modifiers="accel" key="a" command="cmd_selectAll"/>    
  <key id="msiSelectDownKey" keycode="VK_DOWN" command="cmd_selectLineNext" modifiers="shift"/>
  <key id="msiSelectMacroKey" key=" " modifiers="control,accel" command="cmd_macrofragment"/>
  <key id="msiSelectNextKey" keycode="VK_RIGHT" command="cmd_selectCharNext" modifiers="shift"/>
  <key id="msiSelectPrevKey" keycode="VK_LEFT" command="cmd_selectCharPrevious" modifiers="shift"/>
  <key id="msiSelectUpKey" keycode="VK_UP" command="cmd_selectLinePrevious" modifiers="shift"/>
  <key id="msiStructTagKey" key="3" action="document.getElementById('StructTagSelections').focus();" modifiers="accel"/>
  <key id="msiSymbolKeyboardKey" key="k" modifiers="accel"   command="cmd_oneshotSymbol"/>
  <key id="msiTextTagKey" key="1" action="document.getElementById('TextTagSelections').focus();" modifiers="accel"/>
  <key id="msiThinspaceKey" modifiers="accel" key="," oncommand="msiInsertStockSpace('thinSpace');"/>  
  <key id="msiUnderlineKey" modifiers="accel" key="u" oncommand="msiDoStatefulCommand(cmd_texttag','underline');"/>    
  <key id="msiUndoKey" key="z" command="cmd_undo" modifiers="accel"/>
  <key id="msiZoomInKey" key="+" command="cmd_zoomin" modifiers="accel,shift"/>
  <key id="msiZoomOutKey" key="-" command="cmd_zoomout" modifiers="accel"/>
  <key id="msiZoomReset" key="0" command="cmd_zoomreset" modifiers="accel"/>

</keyset>

<!--commands needed for some of these key bindings-->
<commandset>
  <command id="cmd_charPrevious" oncommand="msiGoDoCommand('cmd_charPrevious')"/>
  <command id="cmd_charNext" oncommand="msiGoDoCommand('cmd_charNext')"/>
  <command id="cmd_selectCharPrevious" oncommand="msiGoDoCommand('cmd_selectCharPrevious')" />
  <command id="cmd_selectCharNext" oncommand="msiGoDoCommand('cmd_selectCharNext')" />
  <command id="cmd_linePrevious" oncommand="msiGoDoCommand('cmd_linePrevious')"/>
  <command id="cmd_lineNext" oncommand="msiGoDoCommand('cmd_lineNext')"/>
  <command id="cmd_selectLinePrevious" oncommand="msiGoDoCommand('cmd_selectLinePrevious')"/>
  <command id="cmd_selectLineNext" oncommand="msiGoDoCommand('cmd_selectLineNext')" />
  <command id="cmd_showhelp" oncommand="msiGoDoCommand('cmd_showhelp')"/>
  <command id="cmd_close" oncommand="msiGoDoCommand('cmd_close')"/>
  <command id="cmd_selectAll" oncommand="msiGoDoCommand('cmd_selectAll')"/>
</commandset>
