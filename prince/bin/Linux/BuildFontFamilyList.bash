#!/bin/sh
# build list of family names of ttf and otf fonts
fc-list : file family | grep -e 'ttf' -e 'otf' | sed 's/^.*: //' | sort | uniq - "$*" >$1

