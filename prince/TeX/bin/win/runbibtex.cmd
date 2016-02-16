@echo off
call "%1"

setlocal enabledelayedexpansion
REM $1 is the tex directory we are working in
REM optional "-d <directory>" for a non-standard BIBINPUTS

set BIBINPUTS=
cd %2
shift

:Loop
IF "%2"=="" GOTO Continue
if "%2"=="-d" goto setInputDir
SHIFT
GOTO Loop
:Continue


goto doIt
:setInputDir
shift
set BIBINPUTS="%2"

:doIt

set TEXMF_HOME=%MSITEXMF_HOME%
set path="%MSITEXBIN%";%path%
bibtex main
echo done > sentinel
endlocal
if defined SWPDEBUG pause
