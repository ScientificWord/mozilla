rem @echo off
pushd %2
set path="C:\program files\miktex 2.7\miktex\bin"
for /L %%i in (1,1,%5) do echo Pass %%i & echo. & "xelatex.exe" -job-name=%4 %3 %6 %7 %8 %9
popd