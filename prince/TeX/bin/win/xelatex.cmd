rem @echo off
setlocal enabledelayedexpansion
pushd %1
set path="%MSITEXBIN%"
xelatex %2 %4
echo done > sentinel
popd