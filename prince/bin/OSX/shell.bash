#! /bin/bash
# echo "Calling open $1" > $HOME/shell.log
# open -W $1 >> $HOME/shell.log 2>&1 &
echo 1: $1
echo 2: $2
echo 3: $3
echo 4: $4

eval cd "'"$1"'"
echo `pwd`
open $2 $3 $4

