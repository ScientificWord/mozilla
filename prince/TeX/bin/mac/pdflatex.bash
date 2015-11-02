#!/bin/sh
if [ -f ~/.mackichan/MSITeX.bash ]
then
	source ~/.mackichan/MSITeX.bash
else
	source /Applications/MacKichan/MSITeX.bash
fi
export PATH=$MSITEXBIN:$PATH

eval cd "'"$1"'"
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP", $4 is a spare.
pdflatex -interaction=batchmode -shell-escape $2
echo done > sentinel

