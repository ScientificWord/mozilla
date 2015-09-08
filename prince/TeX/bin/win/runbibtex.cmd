@echo off
setlocal enabledelayedexpansion
# $1 is the tex directory we are working in
# optional "-d <directory>" for a non-standard BIBINPUTS

setx BIBINPUTS=
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
setx BIBINPUTS=%1

:doIt

setx TEXMF_HOME=%MSITEXMF_HOME%
set path="%MSITEXBIN%":%path%
bibtex main
echo done > sentinel
endlocal
