echo off
set T=%MSITEXBIN%
if %T%x==x set T=c:\texlive\2014msi\bin\win32
echo %T%
path=%T%;%windir%\system32
echo %path%
start %1 %2 %3 %4 %5 %6

