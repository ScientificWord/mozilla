#!/bin/sh
pushd $1
if [ "`uname -s`" = "Darwin" ]; then
	export PATH=/usr/texbin:$PATH
fi
xelatex main
echo done > sentinel
popd

