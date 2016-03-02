rem @echo off
call %1
setlocal enabledelayedexpansion
gswin32c -dEPSCrop -sDEVICE=pdfwrite -o %3 %2
if defined SWPDEBUG pause
