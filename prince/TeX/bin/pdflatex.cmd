@echo off
pushd %2
for /L %%i in (1,1,%4) do echo Pass %%i & echo. & "xelatex.exe" %3 %5 %6 %7 %8 %9
popd