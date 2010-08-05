rem @echo off
pushd %2
set path="%MSITEXBIN%"
makeindex %4
echo done > sentinel
popd