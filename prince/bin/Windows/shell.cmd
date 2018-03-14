echo off
if %MSIPATHS%x==x goto skip
call "%MSIPATHS%"
:skip
set T=%MSITEXBIN%
if %T%x==x set T=c:\texlive\2017\bin\win32
path=%T%;%windir%\system32
cd %1
%2 %3 %4 %5 %6
if defined SWPDEBUG pause

