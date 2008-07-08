@echo off
pushd %2
for /L %%i in (1,1,%4) do echo Pass %%i & echo. & "c:\program files\miktex 2.7\miktex\bin\xelatex.exe" %3 %5 %6 %7 %8 %9
popd