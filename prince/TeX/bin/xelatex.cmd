rem @echo off
setlocal enabledelayedexpansion
pushd %2
set path="%MSITEXBIN%"
xelatex -interaction=nonstopmode -jobname=%4 %3 %6 %7 %8 %9
echo done > sentinel
popd