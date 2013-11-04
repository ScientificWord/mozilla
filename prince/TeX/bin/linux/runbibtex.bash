source ~/.mackichan/MSITeX.bash
# $1 is the target directory, $2 is the file to compile, $3 is usually "SWP"
cd $1
while [ $# -gt 0 ]
do
  case "$1" in
    "-d")
	  BIBINPDIR="$2"
	  shift
	  ;;
  esac
  shift
done

export PATH=$MSITEXBIN
export BIBINPUTS=$BIBINPDIR:$BIBINPUTS
echo BIBINPUTS=$BIBINPUTS > ~/log
bibtex main 
echo done > sentinel
exit 0
