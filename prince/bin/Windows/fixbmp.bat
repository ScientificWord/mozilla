REM echo off
setlocal enabledelayedexpansion

cd %1
if "%4x" == "1x" goto :threed
  "%~2\convert.exe" graphics\%3.bmp gcache\%3.png
  goto :theend
:threed
  "%~2\convert.exe" graphics\%3.bmp -crop 66.67%%x+0+0! gcache\%3.png
:theend
pause