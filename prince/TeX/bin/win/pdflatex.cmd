rem @echo off
setlocal enabledelayedexpansion
pushd %1
set path="%MSITEXBIN%"
pdflatex -interaction=nonstopmode -jobname=%3 %2 %4
echo done > sentinel
popd