rem @echo off
setlocal enabledelayedexpansion
pushd %1
set path="%MSITEXBIN%"
pdflatex %2
echo done > sentinel
popd