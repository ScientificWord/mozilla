#!/bin/bash# $1 is a legacy argument, no longer used
pushd $1 
export PATH=/usr/texbin/:$PATH
pdflatex -interaction=nonstopmode -jobname=$3 $2 $4
echo done > sentinel
popd

