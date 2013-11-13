@echo off
setlocal enabledelayedexpansion
cd %1
shift
:checkParams
if "%1"=="-d" goto setInputDir
goto doIt
:setInputDir
shift
setx BIBINPUTS=%1:%BIBINPUTS%
shift
goto checkParams
:doIt

set path="%MSITEXBIN%"
bibtex main
echo done > sentinel
endlocal
