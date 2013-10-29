
pushd "%1"
set path="%MSITEXBIN%"
bibtex main
echo done > sentinel
popd
endlocal
:leave
