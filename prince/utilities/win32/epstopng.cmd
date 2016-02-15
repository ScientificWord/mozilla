rem @echo off
call "%1"
setlocal enabledelayedexpansion
set path="%MSITEXBIN%";%PATH%
gswin32c -dEPSCrop -sDEVICE=pngalpha -o %3 %2
if defined SWPDEBUG pause
