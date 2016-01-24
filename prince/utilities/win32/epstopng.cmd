rem @echo off
setlocal enabledelayedexpansion
set path="%MSITEXBIN%";%PATH%
gswin32c -sDEVICE=pngalpha -o gcache/%1.png graphics/%1.eps
