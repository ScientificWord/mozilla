#!/bin/sh

cd "$2"
source "$1"
dir=`dirname $INKSCAPE`
export  PATH="$dir":$PATH

"$INKSCAPE" -z --shell < wmflist
