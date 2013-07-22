#!/bin/sh

# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP"
cd $1
/usr/texbin/makeindex $3 
touch sentinel
exit 0

