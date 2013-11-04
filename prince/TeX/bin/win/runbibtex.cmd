@echo off
setlocal
:checkParams
if "%1"=="-d" goto setInputDir
goto doIt
:setInputDir
shift
set BIBINPDIR=%1
shift
goto checkParams
:doIt

set path="%MSITEXBIN%"
set BINPUTS=%BIBINPDIR%:%BINPUTS%
bibtex main
echo done > sentinel
endlocal
