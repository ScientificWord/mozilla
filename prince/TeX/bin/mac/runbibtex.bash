#!/bin/sh
source ~/.mackichan/MSITeX.bash
# $1 is the tex directory we are working in
# optional "-d <directory>" for a non-standard BIBINPUTS

export BIBINPUTS=
eval cd "'"$1"'"
shift
while [ $# -gt 0 ]
do
  case "$1" in
    "-d")
	  export BIBINPUTS=$2
	  shift
	  ;;
  esac
  shift
done

export TEXMF_HOME=$MSITEXMF_HOME/
echo BIBINPUTS=$BIBINPUTS > ~/log
bibtex main
echo done > sentinel
exit 0

