@echo off
call %1

setlocal enabledelayedexpansion
REM $1 is the tex directory we are working in
REM optional "-d <directory>" for a non-standard BIBINPUTS

set BIBINPUTS=
cd %2

:Loop
shift
IF "%~2"=="" GOTO Continue
IF "%~2"=="-d" goto setBIBDir
IF "%~2"=="-s" goto setBSTDir
GOTO Loop
:setBIBDir
shift
set BIBINPUTS=%~2
GOTO Loop
:setBSTDir
shift
set BSTINPUTS=%~2
GOTO Loop
:Continue

set TEXMF_HOME=%MSITEXMF_HOME%
set path="%MSITEXBIN%";%path%
bibtex main
echo done > sentinel
echo TEXMF_HOME=%TEXMF_HOME% BIBINPUTS=%BIBINPUTS% > trace
endlocal
if defined SWPDEBUG pause
