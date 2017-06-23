rem @echo off
call "%~1"

cd "%~2"
rem export  PATH="%INKSCAPE"\..:$PATH

"$INKSCAPE" -z --shell < wmflist
