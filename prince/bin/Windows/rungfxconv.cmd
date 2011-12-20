@echo off
setlocal
pushd %targDirectory%
set path="%exepath%";%PATH%
%commandLine%
if not exist %outputFile% echo failed >%outputFile%.txt 
popd
endlocal
