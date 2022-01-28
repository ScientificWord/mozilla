@echo off

echo !!!!!!!!!!!!!!!!!! run checkall.bash instead !!!!!!!!!!!!!!!!!!
echo Running test scripts and comparing with known good version.
FOR %%F in (muptests\health1 muptests\health2) DO CALL check %%F
FOR %%F in (muptests\commands muptests\errors muptests\params muptests\uprefs) DO CALL check %%F

FOR %%F in (muptests\dm10-1 muptests\Statistics1) DO CALL check %%F
FOR %%F in (muptests\Interpret muptests\Interpret2) DO CALL check %%F

FOR %%F in (mpltests\health1 mpltests\params) DO CALL check %%F

FOR %%F in (othertests\SwitchEngines) DO CALL check %%F
