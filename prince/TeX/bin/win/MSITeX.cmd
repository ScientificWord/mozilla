rem @echo off
set MSITEX=@@TeX_Root@@
set MSITEXBIN=@@TeX_Bindir@@
set MSITEXMF=@@TeXmf_Local@@
set MSITEXMF_HOME=%MSITEX%\texmf-dist
set PATH=@@TeX_Bindir@@;@@GSPATH@@;%PATH%
set TEXMFLOCAL=%MSITEXMF%
if exist %programfiles(x86)%\Inkscape set PATH=%programfiles(x86)%\Inkscape;%PATH%
if exist %programfiles%\Inkscape set PATH=%programfiles%\Inkscape;%PATH%
set INKSCAPE=inkscape.com
