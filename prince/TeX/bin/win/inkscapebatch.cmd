#!/bin/sh

cd "$2"
call "$1"
dir=`dirname $INKSCAPE`
export  PATH="$dir":$PATH

"$INKSCAPE" -z --shell < wmflist
