rem @echo off
setlocal enabledelayedexpansion
pushd %2
set path="%MSITEXBIN%";%PATH%
repstopdf --outfile=graphics/%1.pdf graphics/%1.eps
popd
