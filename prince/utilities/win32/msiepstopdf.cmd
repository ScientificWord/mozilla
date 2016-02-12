rem @echo off
call "%MSIPATHS%"
setlocal enabledelayedexpansion
set path="%MSITEXBIN%";%PATH%
gswin32c -dEPSCrop -sDEVICE=pdfwrite -o %2 %1
if defined SWPDEBUG pause
