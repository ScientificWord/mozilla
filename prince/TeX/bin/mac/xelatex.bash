#!/bin/sh
if [ "`uname -s`" = "Darwin" ]; then
	export PATH=/Library/TeX/texbin:$PATH
fi
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP", $4 is a spare.
osascript `dirname "$0"`/pdflatex.scpt $1 $2 $3 xelatex 

