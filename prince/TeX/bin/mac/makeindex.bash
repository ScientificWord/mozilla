#!/bin/sh
source ~/.mackichan/MSITeX.bash
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP"
cd $1
export PATH=$MSITEXBIN
makeindex main 
echo done > sentinel
exit 0

