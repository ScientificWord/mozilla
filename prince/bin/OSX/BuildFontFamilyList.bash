#!/bin/sh
# build list of family names of ttf and otf fonts
/usr/X11/fc-list : file family | /usr/bin/grep -e 'ttf' -e 'otf' | /usr/bin/sed 's/^.*: //' | /usr/bin/sort | /usr/bin/uniq - "$*"

