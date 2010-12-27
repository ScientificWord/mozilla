#!/bin/bash
# $1 is a legacy argument, no longer used
pushd $2 > /Users/barry/pdftex.log
echo "Calling pdflatex -interaction=nonstopmode -jobname=$4 $3 $6 $7 $8 $9"  >> /Users/barry/pdftex.log
/usr/texbin/pdflatex -interaction=nonstopmode -jobname=$4 $3 $6 $7 $8 $9 >> /Users/barry/pdftex.log 2>&1
echo done > sentinel
popd

