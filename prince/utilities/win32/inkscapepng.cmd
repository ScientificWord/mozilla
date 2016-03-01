

call "%1"
set  PATH=%MSITEXBIN%:%PATH%

"%INKSCAPE%" -z --export-png="%3" --file "%2"


