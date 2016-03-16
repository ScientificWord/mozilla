#!/bin/sh
source "$1"
date > ~/msi.log
echo MacKichan Software PDF compile log >> ~/msi.log
export PATH=$MSITEXBIN:$PATH
echo PATH: $PATH >> ~/msi.log
cd "$2"
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
# $1 is location of MSITeX.bash, $2 is the target directory, $3 is the file to compile, $4 is usually "SWP", $5 is a spare.
pdflatex -interaction=batchmode -shell-escape $3 >> ~/msi.log 2>&1
echo done > sentinel
echo Working directory files: >> ~/msi.log
ls -l  >> ~/msi.log
echo done >> ~/msi.log
