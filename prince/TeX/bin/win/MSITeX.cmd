rem @echo off
set MSITEX=@@TeX_Root@@
set MSITEXBIN=@@TeX_Bindir@@
set MSITEXMF=@@TeXmf_Local@@
set MSITEXMF_HOME=%MSITEX%\texmf-dist
set PATH=@@TeX_Bindir@@;@@GSPATH@@;%PATH%
set TEXMFLOCAL=%MSITEXMF%
if EXIST %programfiles(x86)%\Inkscape (
	 set INKSCAPE=%programfiles(x86)%\Inkscape\inkscape.com
) ELSE (
	if EXIST %programfiles%\Inkscape (set INKSCAPE=%programfiles%\Inkscape\inkscape.com)
)
