#!/bin/sh
source ~/.mackichan/MSITeX.bash
export PATH=$MSITEXBIN:$PATH

eval cd "'"$1"'"
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP", $4 is a spare.
pdflatex -interaction=batchmode -shell-escape $2
echo done > sentinel

