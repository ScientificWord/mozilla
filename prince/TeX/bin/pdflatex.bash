#!/bin/bash
# $1 is a legacy argument, no longer used
pushd $2 > $HOME/pdftex.log
export PATH=/usr/texbin/:$PATH
echo "Calling pdflatex -interaction=nonstopmode -jobname=$4 $3 $6 $7 $8 $9"  >> $HOME/pdftex.log
pdflatex -interaction=nonstopmode -jobname=$4 $3 $6 $7 $8 $9 >> $HOME/pdftex.log 2>&1
echo done > sentinel
popd

