#!/bin/sh
pushd $1
if [ "`uname -s`" = "Darwin" ]; then
	export PATH=/usr/texbin:$PATH
fi
xelatex -jobname=$3 $2 $4
echo done > sentinel
popd

