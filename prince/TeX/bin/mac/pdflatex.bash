#!/bin/sh
if [ -f ~/.mackichan/MSITeX.bash ]
then
	source ~/.mackichan/MSITeX.bash
else
	source /Applications/MacKichan/MSITeX.bash
fi

date > ~/msi.log
echo MacKichan Software PDF compile log >> ~/msi.log
export PATH=$MSITEXBIN:$PATH
echo PATH: $PATH >> ~/msi.log
# eval cd  "'"$1"'"
cd "$1"
echo pdflatex location: >> ~/msi.log
which pdflatex >> ~/msi.log
echo Current working directory: >> ~/msi.log
pwd >> ~/msi.log
echo pdflatex.bash parameters: >> ~/msi.log
echo "'"$1"'" >> ~/msi.log
echo "'"$2"'" >> ~/msi.log
echo "'"$3"'" >> ~/msi.log
echo "'"$4"'" >> ~/msi.log
echo pdflatex output: >> ~/msi.log
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP", $4 is a spare.
pdflatex -interaction=batchmode -shell-escape $2 >> ~/msi.log 2>&1
echo done > sentinel
echo Working directory files: >> ~/msi.log
ls -l $1 >> ~/msi.log
echo done >> ~/msi.log
