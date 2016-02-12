rem @echo off
call "%MSIPATHS%"
setlocal enabledelayedexpansion
pushd %1
set path="%MSITEXBIN%"
xelatex %2
echo done > sentinel
popd
if defined SWPDEBUG pause
