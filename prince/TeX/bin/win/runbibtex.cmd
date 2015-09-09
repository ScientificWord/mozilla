@echo off
setlocal enabledelayedexpansion
REM $1 is the tex directory we are working in
REM optional "-d <directory>" for a non-standard BIBINPUTS

echo runbibtex

set BIBINPUTS=
cd %1
shift

:Loop
IF "%1"=="" GOTO Continue
if "%1"=="-d" goto setInputDir
SHIFT
GOTO Loop
:Continue


goto doIt
:setInputDir
shift
set BIBINPUTS=%1

:doIt

set TEXMF_HOME=%MSITEXMF_HOME%
set path="%MSITEXBIN%":%path%

echo BIBINPUTS=%BIBINPUTS%
echo PATH=%PATH%
bibtex main
echo done > sentinel
set /p name= Press return to continue
endlocal
