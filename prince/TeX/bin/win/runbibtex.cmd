@echo off
call "%MSIPATHS%"

setlocal enabledelayedexpansion
REM $1 is the tex directory we are working in
REM optional "-d <directory>" for a non-standard BIBINPUTS

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
set path="%MSITEXBIN%";%path%
bibtex main
echo done > sentinel
endlocal
if defined SWPDEBUG pause
