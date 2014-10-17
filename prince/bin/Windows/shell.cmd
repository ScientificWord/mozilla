
echo off
set T=%TEXBINDIR%
if %T%x==x set T=c:\texlive\2014msi\bin\win32
path=%T%;%windir%\system32
start %1 %2 %3 %4 %5 %6

