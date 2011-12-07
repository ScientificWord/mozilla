@echo off
setlocal
pushd %targDirectory%
set path="%exepath%";%PATH%
%commandLine%
popd
endlocal
