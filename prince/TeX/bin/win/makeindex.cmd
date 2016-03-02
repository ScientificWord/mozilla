rem @echo off
call %1
pushd "%2"
set path="%MSITEXBIN%";%PATH%
makeindex main
echo done > sentinel
popd
if defined SWPDEBUG pause
