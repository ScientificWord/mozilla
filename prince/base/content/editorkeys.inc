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
  <key id="msiDirectPrintKey" key="p" oncommand="msiGoDoCommand('cmd_printDirect');" modifiers="accel"/>
  <key id="msiFindKey" key="f" command="cmd_find" modifiers="accel"/>
  <key id="msiFindNextKey" keycode="VK_F3" keytext="F3" command="cmd_findNext" />
  <key id="msiFindPrevKey" keycode="VK_F3" command="cmd_findPrev" modifiers="shift"/>
<!--   <key id="msiGreekKeyboardKey" key="g" modifiers="accel"   command="cmd_oneshotGreek"/> -->
<!--   <key id="msiMacHelp" key="?" modifiers="accel,shift" command="cmd_showhelp"/>
 -->  
  <key id="msiNewFileKey" key="n" modifiers="accel" oncommand="openNewDocument();"/>
  <key id="msiOpenFileKey" key="o" modifiers="accel" oncommand = "openDocument();" />
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
<!--   <key id="msiSymbolKeyboardKey" key="k" modifiers="accel"   command="cmd_oneshotSymbol"/> -->
  <key id="msiThinspaceKey" modifiers="accel" key="," oncommand="msiInsertStockSpace('thinSpace');"/>
  <key id="msiTypesetPrintKey" key="p" oncommand="msiGoDoCommand('cmd_printPdf');" modifiers="accel,shift"/>
  <key id="msiUnderlineKey" modifiers="accel" key="u" oncommand="msiDoStatefulCommand('cmd_texttag','underline');"/>
  <key id="msiUndoKey" key="z" command="cmd_undo" modifiers="accel"/>
  <key id="msiZoomInKey" key="+" command="cmd_zoomin" modifiers="accel,shift"/>
  <key id="msiZoomOutKey" key="-" command="cmd_zoomout" modifiers="accel"/>
  <key id="msiZoomReset" key="0" command="cmd_zoomreset" modifiers="accel"/>
      <key event="keypress" key="`" modifiers="accel"       oncommand="msiRevCharQuick('&#x0300;');"/>
      <key event="keypress" key="'" modifiers="accel"       oncommand="msiRevCharQuick('&#x0301;');"/>
      <key event="keypress" key="^" modifiers="accel,shift" oncommand="msiRevCharQuick('&#x0302;');"/>
      <key event="keypress" key="~" modifiers="accel,shift" oncommand="msiRevCharQuick('&#x0303;');"/>
      <key event="keypress" key='_' modifiers="accel,shift" oncommand="msiRevCharQuick('&#x0305;');"/>
      <key event="keypress" key="." modifiers="accel"       oncommand="msiRevCharQuick('&#x0307;');"/>
      <key event="keypress" key='"' modifiers="accel,shift" oncommand="msiRevCharQuick('&#x0308;');"/>
      <key event="keypress" key='!' modifiers="accel,shift" oncommand="msiRevCharQuick('&#x20d7;');"/>
</keyset>

