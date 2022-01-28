@echo off

echo !!!!!!!!!!!!!!!!!! run checknew.bash instead !!!!!!!!!!!!!!!!!!
echo Running test scripts and comparing with known good version.
FOR %%F in (muptests\Cleanup muptests\Fixup muptests\Fixup2 muptests\FixupCalc muptests\FixupBugs) DO CALL check %%F
FOR %%F in (muptests\FixupDefs0 muptests\FixupDefs1 muptests\FixupDefs2 muptests\FixupDefs3) DO CALL check %%F
FOR %%F in (muptests\Operators muptests\Interpret3) DO CALL check %%F
