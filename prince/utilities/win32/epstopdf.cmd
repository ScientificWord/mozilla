rem @echo off
setlocal enabledelayedexpansion
set path="%MSITEXBIN%";%PATH%
gswin32c -dEPSCrop -sDEVICE=pdfwrite -o %2 %1
