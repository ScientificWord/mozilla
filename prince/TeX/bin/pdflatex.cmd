rem @echo off
pushd %2
set path="%MSITEXBIN%"
xelatex -jobname=%4 %3 %6 %7 %8 %9
echo done > sentinel
popd