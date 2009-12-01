#! /bin/bash

echo Running test scripts and comparing with known good version.
for F in muptests/Cleanup muptests/Fixup muptests/Fixup2 muptests/FixupCalc muptests/FixupBugs ; do bash check.bash $F ; done
for F in muptests/FixupDefs0 muptests/FixupDefs1 muptests/FixupDefs2 muptests/FixupDefs3 ; do bash check.bash $F ; done

for F in muptests/Operators muptests/Interpret3 ; do bash  check.bash $F ; done
