rem @echo off
setlocal enabledelayedexpansion
set path="%MSITEXBIN%";%PATH%
gswin32c -dEPSCrop -sDEVICE=pngalpha -o %2 %1
