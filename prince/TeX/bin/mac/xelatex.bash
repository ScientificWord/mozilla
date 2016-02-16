#!/bin/sh

source "$1"
# $1 is the path of the MSITeX.bash file, $2 is the target directory, $3 is the file to compile, $4 is usually "SWP", $5 is a spare.
osascript `dirname "$0"`/pdflatex.scpt $2 $3 $4 xelatex 

