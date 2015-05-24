rem @echo off
setlocal enabledelayedexpansion
pushd %1
set path="%MSITEXBIN%"
pdflatex -interaction=batchmode -shell-escape %2
echo done > sentinel
popd
