#!/bin/sh
pushd $1
source ~/.mackichan/MSITeX.bash
export PATH=$MSITEXBIN:$PATH

xelatex main
echo done > sentinel
popd

