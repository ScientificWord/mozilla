rem @echo off
pushd %2
set path="%programfiles%\texlive\2009\bin\win32"
makeindex %4
echo done > sentinel
popd