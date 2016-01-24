rem @echo off
setlocal enabledelayedexpansion
set path="%MSITEXBIN%";%PATH%
gswin32c -sDEVICE=pdfwrite -o graphics/%1.pdf graphics/%1.eps
