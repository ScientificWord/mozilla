
call "%~1"
cd "%INKSCAPEPATH%"


"inkscape.exe" -z --shell <"%~2\wmflist"
