#!/bin/sh

source "$1"
# $2 is the tex directory we are working in
# optional "-d <directory>" for a non-standard BIBINPUTS

export BIBINPUTS=
cd "$2"
shift
while [ $# -gt 0 ]
do
  case "$2" in
    "-d")
	  export BIBINPUTS=$3
	  shift
	  ;;
  esac
  shift
done

export TEXMF_HOME=$MSITEXMF_HOME
echo BIBINPUTS=$BIBINPUTS > ~/log
bibtex main
echo done > sentinel
exit 0

