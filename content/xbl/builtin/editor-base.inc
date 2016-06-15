<!--       <handler event="keypress" key=" " modifiers="shift" command="cmd_scrollPageUp" />
      <handler event="keypress" key=" " command="cmd_scrollPageDown" />-->
      <handler event="keypress" keycode="VK_LEFT" command="cmd_charPrevious"/>
      <handler event="keypress" keycode="VK_RIGHT" command="cmd_charNext"/>
      <handler event="keypress" keycode="VK_LEFT" command="cmd_selectCharPrevious" modifiers="shift"/>
      <handler event="keypress" keycode="VK_RIGHT" command="cmd_selectCharNext" modifiers="shift"/>
      <handler event="keypress" keycode="VK_UP" command="cmd_linePrevious"/>
      <handler event="keypress" keycode="VK_DOWN" command="cmd_lineNext"/>
      <handler event="keypress" keycode="VK_UP" command="cmd_selectLinePrevious" modifiers="shift"/>
      <handler event="keypress" keycode="VK_DOWN" command="cmd_selectLineNext" modifiers="shift"/>
      <handler event="keypress" key="z" command="cmd_MSIundo" modifiers="accel"/>
      <handler event="keypress" key="z" command="cmd_MSIredo" modifiers="accel,shift" />
      <handler event="keypress" key="x" command="cmd_MSIcut" modifiers="accel"/>
      <handler event="keypress" key="c" command="cmd_MSIcopy" modifiers="accel"/>
      <handler event="keypress" key="v" command="cmd_MSIpaste" modifiers="accel"/>
      <handler event="keypress" key="v" command="cmd_pasteNoFormatting" modifiers="accel,shift"/>
      <handler event="keypress" key="1" action="focusOn('TextTagSelections')" modifiers="accel"/>
      <handler event="keypress" key="2" action="focusOn('ParaTagSelections')" modifiers="accel"/>
      <handler event="keypress" key="3" action="focusOn('StructTagSelections')" modifiers="accel"/>
      <handler event="keypress" key="4" action="focusOn('FrontMTagSelections')" modifiers="accel"/>
      <handler event="keypress" keycode="VK_ENTER" modifiers="shift" command="cmd_insertBreakCommand"/>
      <handler event="keypress" keycode="VK_RETURN" modifiers="shift" command="cmd_insertBreakCommand"/>



