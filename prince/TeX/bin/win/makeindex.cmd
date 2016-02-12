rem @echo off
call "%MSIPATHS%"
pushd %1
set path="%MSITEXBIN%";%PATH%
makeindex main
echo done > sentinel
popd
if defined SWPDEBUG pause
