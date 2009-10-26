rem @echo off
pushd %2
set path="C:\program files\miktex 2.8\miktex\bin"
"xelatex.exe" -job-name=%4 %3 %6 %7 %8 %9
popd