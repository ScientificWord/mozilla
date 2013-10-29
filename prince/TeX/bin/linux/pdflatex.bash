#!/bin/bash
cd $1 
export PATH=/usr/local/texlive/swTexbin/
pdflatex main
echo done > sentinel


