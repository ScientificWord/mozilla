
REM echo off
setlocal enabledelayedexpansion
cd %1
"%~2\convert.exe" graphics\%3.bmp -crop 80%%x+0+0 gcache\%3.png
