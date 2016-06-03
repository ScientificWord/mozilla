      <handler event="keypress" key=" " modifiers="shift" command="cmd_scrollPageUp" />
      <handler event="keypress" key=" " command="cmd_scrollPageDown" />

      <handler event="keypress" keycode="VK_UP" command="cmd_linePrevious" />
      <handler event="keypress" keycode="VK_DOWN" command="cmd_lineNext" />
      <handler event="keypress" keycode="VK_LEFT" command="cmd_charPrevious" />
      <handler event="keypress" keycode="VK_RIGHT" command="cmd_charNext" />

      <handler event="keypress" key="x" command="cmd_MSIcut" modifiers="accel"/>
      <handler event="keypress" key="c" command="cmd_MSIcopy" modifiers="accel"/>
      <handler event="keypress" key="v" command="cmd_MSIpaste" modifiers="accel"/>
      <handler event="keypress" key="z" command="cmd_MSIundo" modifiers="accel"/>
      <handler event="keypress" key="z" command="cmd_MSIredo" modifiers="accel,shift" />
      <handler event="keypress" key="a" command="cmd_MSIselectAll" modifiers="accel"/>
