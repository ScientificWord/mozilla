rem @echo off
pushd %1
set path="%MSITEXBIN%"
makeindex main
echo done > sentinel
popd