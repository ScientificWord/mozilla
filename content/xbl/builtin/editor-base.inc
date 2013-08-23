<!--       <handler event="keypress" key=" " modifiers="shift" command="cmd_scrollPageUp" />
      <handler event="keypress" key=" " command="cmd_scrollPageDown" />
 -->
      <handler event="keypress" keycode="VK_LEFT" command="cmd_charPrevious"/>
      <handler event="keypress" keycode="VK_RIGHT" command="cmd_charNext"/>
      <handler event="keypress" keycode="VK_LEFT" command="cmd_selectCharPrevious" modifiers="shift"/>
      <handler event="keypress" keycode="VK_RIGHT" command="cmd_selectCharNext" modifiers="shift"/>
      <handler event="keypress" keycode="VK_UP" command="cmd_linePrevious"/>
      <handler event="keypress" keycode="VK_DOWN" command="cmd_lineNext"/>
      <handler event="keypress" keycode="VK_UP" command="cmd_selectLinePrevious" modifiers="shift"/>
      <handler event="keypress" keycode="VK_DOWN" command="cmd_selectLineNext" modifiers="shift"/>
      <handler event="keypress" key="z" command="cmd_undo" modifiers="accel"/>
      <handler event="keypress" key="z" command="cmd_redo" modifiers="accel,shift" />
      <handler event="keypress" key="x" command="cmd_cut" modifiers="accel"/>
      <handler event="keypress" key="c" command="cmd_copy" modifiers="accel"/>
      <handler event="keypress" key="v" command="cmd_paste" modifiers="accel"/>
      <handler event="keypress" key="v" command="cmd_pasteNoFormatting" modifiers="accel,shift"/>
      <handler event="keypress" key="1" action="focusOn('TextTagSelections')" modifiers="accel"/>
      <handler event="keypress" key="2" action="focusOn('ParaTagSelections')" modifiers="accel"/>
      <handler event="keypress" key="3" action="focusOn('StructTagSelections')" modifiers="accel"/>
      <handler event="keypress" key="4" action="focusOn('FrontMTagSelections')" modifiers="accel"/>     
      <handler event="keypress" key="`" modifiers="accel" action="msiRevCharQuick('&#x0300;');"/>
      <handler event="keypress" key="'" modifiers="accel" action="msiRevCharQuick('&#x0301;');"/>
      <handler event="keypress" key="^" modifiers="accel,shift" action="msiRevCharQuick('&#x0302;');"/>
      <handler event="keypress" key="~" modifiers="accel,shift" action="msiRevCharQuick('&#x0303;');"/>
      <handler event="keypress" key='_' modifiers="accel,shift" action="msiRevCharQuick('&#x0305;');"/>
      <handler event="keypress" key="." modifiers="accel" action="msiRevCharQuick('&#x0307;');"/>
      <handler event="keypress" key='"' modifiers="accel,shift" action="msiRevCharQuick('&#x0308;');"/>

