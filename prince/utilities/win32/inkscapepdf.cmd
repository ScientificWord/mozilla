
set  PATH=%MSITEXBIN%:%PATH%

call "%1"
echo %1
"%INKSCAPE%" -z --export-pdf="%3" --file "%2"


