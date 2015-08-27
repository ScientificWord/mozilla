#!/bin/sh
source ~/.mackichan/MSITeX.bash
export PATH=$MSITEXBIN:$PATH

echo $1
eval cd "'"$1"'"
echo `pwd`
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP", $4 is a spare.
# cmd="osascript "`dirname "$0"`"/pdflatex.scpt "$1" "$2" "$3" pdflatex"
# echo "command is: "$cmd
# osascript `dirname "$0"`/pdflatex.scpt "$1" $2 $3 pdflatex
pdflatex -interaction=batchmode -shell-escape $2
echo done > sentinel

