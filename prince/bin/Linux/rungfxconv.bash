pushd $targDirectory
export PATH=$exepath:$PATH
$commandLine
if [ ! -e $outputFile ] echo failed > ${outputFile}.txt
popd