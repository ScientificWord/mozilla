pushd $2
export PATH=$MSITEXBIN:$PATH
makeindex $4
echo done > sentinel
popd