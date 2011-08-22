setlocal enabledelayedexpansion
REM echo 1=%1, 2=%2, 3=%3, 4=%4, 5=%5, 6=%6
cd "%1"
fc-cache
otfinfo -a %windir%\fonts\*.?tf > %2