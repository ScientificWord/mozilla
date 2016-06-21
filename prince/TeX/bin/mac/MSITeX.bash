#! /bin/bash
if [ -e ~/.tex/tlyear ]
	then
	tlyear=`cat ~/.tex/tlyear`
else
	if [ -e /usr/local/texlive/tlyear ]
		then
		tlyear=`cat /usr/local/texlive/tlyear`
	else
		tlyear='2015'
	fi
fi

export MSITEX=/usr/local/texlive/$tlyear
export MSITEXBIN=/Library/TeX/texbin
export MSITEXMF=/usr/local/texlive/texmf-local
export MSITEXMF_HOME=$MSITEX/texmf-dist
export PATH=$MSITEXBIN:/usr/local/bin:$PATH
export TEXMFLOCAL=$MSITEXMF
export INKSCAPE=/Applications/Inkscape.app/Contents/Resources/bin/inkscape

