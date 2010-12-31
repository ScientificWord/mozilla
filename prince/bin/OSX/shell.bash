#! /bin/bash
echo "Calling open $1" > /Users/barry/shell.log
open -W $1 >> shell.log 2>&1

