#! /bin/bash

echo Running test scripts and comparing with known good version.
for F in muptests/health1 muptests/health2 ; do bash check.bash $F ; done
for F in muptests/Commands muptests/Errors muptests/params muptests/uprefs ; do bash check.bash $F ; done

for F in muptests/dm10-1 muptests/Statistics1 ; do bash  check.bash $F ; done
for F in muptests/Interpret muptests/Interpret2 ; do bash check.bash $F ; done

if [ $OSTYPE == cygwin ]; then
  for F in mpltests/health1 mpltests/params ; do bash check.bash $F ; done
  for F in othertests/SwitchEngines ; do bash check.bash $F ; done ;
fi
  
