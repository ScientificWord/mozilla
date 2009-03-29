#!/bin/sh
# $1 is a legacy argument, no longer used
pushd $2
infile=$3
declare -i j=0 passes=$4 k=0
while [ $j -lt $passes ] 
do
	let k=j+1
	echo Pass $k
	xelatex $infile
	let j=j+1
done
popd

