rem @echo off
call %1
setlocal enabledelayedexpansion
pushd "%2"
set path="%MSITEXBIN%"
xelatex "%3"
echo done > sentinel
popd
if defined SWPDEBUG pause
