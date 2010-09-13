#!/bin/sh
# $1 is a legacy argument, no longer used
pushd $2
xelatex -interaction=nonstopmode -jobname=$4 $3 $6 $7 $8 $9
echo done > sentinel
popd

