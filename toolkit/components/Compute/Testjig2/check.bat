@echo off
setlocal
set C=TestJig2
set D=diff -w

set base=TestJigData\%1

echo =============== checking %base%.tscript ===============

pushd ..
%C% %base%.tscript >>TestJigData\stderr.log 2>&1
sed -f TestJigData\RemoveNoise.sed %base%.xhtml >%base%.tmp
%D% %base%.tmp %base%.OK
popd

:done

