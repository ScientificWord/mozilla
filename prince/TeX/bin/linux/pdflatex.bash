#!/bin/bash
pushd $1 
export PATH=/usr/local/texlive/swTexbin/:$PATH
pdflatex -jobname=$3 $2 $4
echo done > sentinel
popd

