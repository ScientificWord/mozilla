rem @echo off
setlocal enabledelayedexpansion
pushd %1
set path="@@TeX_Bindir@@"
xelatex -jobname=%3 %2 %4
echo done > sentinel
popd