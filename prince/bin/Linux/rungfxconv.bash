pushd $targDirectory
export PATH=$exepath:$PATH
$commandLine
if [ ! -e $outputFile ] then
  echo failed > ${outputFile}.txt
fi
popd