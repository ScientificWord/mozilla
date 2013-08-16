rem @echo off
setlocal enabledelayedexpansion
pushd %1
set path="@@TeX_Bindir@@"
pdflatex -jobname=%3 %2 %4
echo done > sentinel
popd