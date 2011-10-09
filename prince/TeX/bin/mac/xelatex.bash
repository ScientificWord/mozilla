#!/bin/sh
if [ "`uname -s`" = "Darwin" ]; then
	export PATH=/usr/texbin:$PATH
fi
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP", $4 is a spare.
osascript `dirname "$0"`/xelatex.applescript $1 $2 $3 $4 

