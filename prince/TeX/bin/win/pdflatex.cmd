rem @echo off
call "%MSIPATHS%"
setlocal enabledelayedexpansion
pushd %1
set path="%MSITEXBIN%";%path%
pdflatex -interaction=batchmode -shell-escape %2
echo done > sentinel
popd
if defined SWPDEBUG pause
