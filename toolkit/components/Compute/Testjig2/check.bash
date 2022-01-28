#! /bin/bash

base=TestJigData/$1

echo =============== checking $base.tscript ===============

pushd .. >/dev/null
./Testjig2 $base.tscript >>TestJigData/stderr.log 2>&1
sed -f TestJigData/RemoveNoise.sed $base.xhtml >$base.tmp
diff -w $base.tmp $base.OK
popd >/dev/null
