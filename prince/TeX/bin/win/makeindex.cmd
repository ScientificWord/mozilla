rem @echo off
pushd %1
set path="%MSITEXBIN%"
makeindex %3
echo done > sentinel
popd