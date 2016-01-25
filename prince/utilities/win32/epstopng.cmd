rem @echo off
echo %1
echo %2
setlocal enabledelayedexpansion
set path="%MSITEXBIN%";%PATH%
gswin32c -dEPSCrop -sDEVICE=pngalpha -o %2 %1
