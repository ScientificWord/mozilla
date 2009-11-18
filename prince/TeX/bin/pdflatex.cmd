rem @echo off
pushd %2
set path="%programfiles%\texlive\2009\bin\win32"
xelatex -jobname=%4 %3 %6 %7 %8 %9
popd