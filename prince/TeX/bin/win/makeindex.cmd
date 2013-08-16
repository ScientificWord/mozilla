rem @echo off
pushd %1
set path="@@TeX_Bindir@@"
makeindex %3
echo done > sentinel
popd