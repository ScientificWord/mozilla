#!/bin/sh

source ~/.mackichan/MSITeX.bash
export PATH=$MSITEXBIN:$PATH

# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP", $4 is a spare.
cmd="osascript "`dirname "$0"`"/pdflatex.scpt "$1" "$2" "$3" pdflatex"
echo "command is: "$cmd
osascript `dirname "$0"`/pdflatex.scpt $1 $2 $3 pdflatex
exit 0

