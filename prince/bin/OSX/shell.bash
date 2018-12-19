#! /bin/bash
echo "Calling open $2" > $HOME/shell.log
# open -W $1 >> $HOME/shell.log 2>&1 &
# echo 1: $1
# echo 2: $2
# echo 3: $3
# echo 4: $4
echo "1\n" > $HOME/shell.log
eval cd "'"$1"'"
echo "2\n" > $HOME/shell.log

# echo `pwd`
open $2 $3 $4

