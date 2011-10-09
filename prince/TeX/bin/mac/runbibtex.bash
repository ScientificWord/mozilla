while [ $# -gt 0 ]
do
  case "$1" in
    "-x")
	  export MSITEXBIN="$2"
	  shift
	  ;;
    "-s")
	  export BSTINPUTS="$2\\\\"
	  shift
	  ;;
    "-b")
	  export BIBINPUTS="$2\\\\"
	  shift
	  ;;
    "-d")
	  BIBTARGDIR="$2"
	  shift
	  ;;
	*)
	  BIBTARGFILE="$1"
	  ;;
  esac
  shift
done
pushd $BIBTARGDIR
export PATH=$MSITEXBIN:$PATH
#echo BIBINPUTS "$BIBINPUTS", BSTINPUTS "$BSTINPUTS", BIBTARGDIR "$BIBTARGDIR", MSITEXBIN "$MSITEXBIN"
#echo bibtex "$BIBTARGFILE"
bibtex "$BIBTARGFILE"
echo done > sentinel
popd