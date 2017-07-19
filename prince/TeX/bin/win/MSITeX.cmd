rem @echo off
if EXIST %USERPROFILE%\.tex\tlyear (
     set /P tlyear=<%USERPROFILE%\.tex\tlyear
) ELSE (
	 if EXIST c:\texlive\tlyear (
         set /P tlyear=<c:\texlive\tlyear
	 ) ELSE (
	     set tlyear=2015
	 )
)
if NOT EXIST "%USERPROFILE%\.tex" (
    mkdir "%USERPROFILE%\.tex"
)
echo | set /p dummyname="%tlyear%" > "%USERPROFILE%\.tex\tlyear"
set MSITEX=C:\texlive\%tlyear%
set MSITEXBIN=C:\texlive\%tlyear%\bin\win32
set MSITEXMF=C:\texlive\texmf-local
set MSITEXMF_HOME=%MSITEX%\texmf-dist
set PATH=C:\texlive\%tlyear%\bin\win32;C:\Program Files (x86)\gs\gs9.16\bin;%PATH%
set TEXMFLOCAL=%MSITEXMF%
if EXIST %programfiles(x86)%\Inkscape (
	 set INKSCAPE=%programfiles(x86)%\Inkscape\inkscape.com
	 set INKSCAPEPATH=%programfiles(x86)%\Inkscape
) ELSE (
	if EXIST %programfiles%\Inkscape (
		set INKSCAPE=%programfiles%\Inkscape\inkscape.com
		set INKSCAPEPATH=%programfiles%\Inkscape
	)
)
