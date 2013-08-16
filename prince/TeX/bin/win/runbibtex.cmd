@echo off
setlocal
:checkParams
if "%1"=="" goto doIt
if "%1"=="-x" goto setExePath
if "%1"=="-b" goto setDBDir
if "%1"=="-s" goto setStyleDir
if "%1"=="-d" goto setTargDir
set BIBTARGFILE=%1
shift
goto checkParams
:setExePath
shift
set MSITEXBIN=%1
shift
goto checkParams
:setDBDir
shift
set BIBINPUTS=%1
shift
goto checkParams
:setStyleDir
shift
set BSTINPUTS=%1
shift
goto checkParams
:setTargDir
shift
set BIBTARGDIR=%1
shift
goto checkParams
:doIt
pushd "%BIBTARGDIR%"
set path="@@TeX_Bindir@@"
bibtex "%BIBTARGFILE%"
echo done > sentinel
popd
endlocal
:leave
