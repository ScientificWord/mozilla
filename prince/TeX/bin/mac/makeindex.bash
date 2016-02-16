#!/bin/sh
source "$1"
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP"
cd "$2"
export PATH=$MSITEXBIN:$PATH
makeindex main
echo done > sentinel
exit 0

