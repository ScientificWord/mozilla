rem @echo off
if EXIST %USERPROFILE%\.tex\tlyear (
     set /P tlyear=<%USERPROFILE%\.tex\tlyear
) ELSE (
     set /P tlyear=<c:\texlive\tlyear
)
set MSITEX=C:\texlive\%tlyear%
set MSITEXBIN=C:\texlive\%tlyear%\bin\win32
set MSITEXMF=C:\texlive\texmf-local
set MSITEXMF_HOME=%MSITEX%\texmf-dist
set PATH=C:\texlive\%tlyear%\bin\win32;C:\Program Files (x86)\gs\gs9.16\bin;%PATH%
set TEXMFLOCAL=%MSITEXMF%
set INKSCAPE=%programfiles%\Inkscape\inkscape.com

