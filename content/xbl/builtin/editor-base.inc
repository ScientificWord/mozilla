      <handler event="keypress" keycode="VK_LEFT" command="cmd_charPrevious"/>
      <handler event="keypress" keycode="VK_RIGHT" command="cmd_charNext"/>
      <handler event="keypress" keycode="VK_LEFT" command="cmd_selectCharPrevious" modifiers="shift"/>
      <handler event="keypress" keycode="VK_RIGHT" command="cmd_selectCharNext" modifiers="shift"/>
      <handler event="keypress" keycode="VK_UP" command="cmd_linePrevious"/>
      <handler event="keypress" keycode="VK_DOWN" command="cmd_lineNext"/>
      <handler event="keypress" keycode="VK_UP" command="cmd_selectLinePrevious" modifiers="shift"/>
      <handler event="keypress" keycode="VK_DOWN" command="cmd_selectLineNext" modifiers="shift"/>
      <handler event="keypress" keycode="VK_TAB" command="cmd_insertSubstructure" modifiers="control"/>
      <handler event="keypress" keycode="VK_F3" command="cmd_findNext" />
      <handler event="keypress" keycode="VK_F3" command="cmd_findPrev" modifiers="shift"/>
      <handler event="keypress" key="z" command="cmd_undo" modifiers="accel"/>
      <handler event="keypress" key="z" command="cmd_redo" modifiers="accel,shift" />
      <handler event="keypress" key="x" command="cmd_cut" modifiers="accel"/>
      <handler event="keypress" key="c" command="cmd_copy" modifiers="accel"/>
      <handler event="keypress" key="v" command="cmd_paste" modifiers="accel"/>
      <handler event="keypress" key="1" action="document.getElementById('TextTagSelections').focus();" modifiers="alt"/>
      <handler event="keypress" key="2" action="document.getElementById('ParaTagSelections').focus();" modifiers="alt"/>
      <handler event="keypress" key="3" action="document.getElementById('StructTagSelections').focus();" modifiers="alt"/>
      <handler event="keypress" key="4" action="document.getElementById('OtherTagSelections').focus();" modifiers="alt"/>
      <handler event="keypress" key="+" action="ZoomManager.prototype.getInstance().enlarge();" modifiers="accel,shift"/>
      <handler event="keypress" key="-" action="ZoomManager.prototype.getInstance().reduce();" modifiers="accel"/>
      <handler event="keypress" key="0" action="ZoomManager.prototype.getInstance().reset();" modifiers="accel"/>
