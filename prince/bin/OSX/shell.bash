#! /bin/bash
echo "Calling open $1" > $HOME/shell.log
open -W $1 >> $HOME/shell.log 2>&1 &

