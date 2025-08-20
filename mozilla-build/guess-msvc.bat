REM -*- Mode: fundamental; tab-width: 8; indent-tabs-mode: 1 -*-
REM @ECHO OFF

set CYGWIN=
if not defined MOZ_NO_RESET_PATH (
    set PATH=%SystemRoot%\System32;%SystemRoot%;%SystemRoot%\System32\Wbem
)

REM if DISPLAY is set, rxvt attempts to load libX12.dll and fails to start
REM (see mozilla bug 376828)
SET DISPLAY=

SET INCLUDE=
SET LIB=

SET WINCURVERKEY=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion
REG QUERY "%WINCURVERKEY%" /v "ProgramFilesDir (x86)" >nul 2>nul
if %ERRORLEVEL% EQU 0 (
  SET WIN64=1
) else (
  SET WIN64=0
)

if "%WIN64%" == "1" (
  SET MSVCROOTKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\VisualStudio
  SET MSVCEXPROOTKEY=HKLM\SOFTWARE\Wow6432Node\Microsoft\VCExpress
) else (
  SET MSVCROOTKEY=HKLM\SOFTWARE\Microsoft\VisualStudio
  SET MSVCEXPROOTKEY=HKLM\SOFTWARE\Microsoft\VCExpress
)

SET MSVC71KEY=%MSVCROOTKEY%\7.1\Setup\VC
SET MSVC8KEY=%MSVCROOTKEY%\8.0\Setup\VC
SET MSVC8EXPRESSKEY=%MSVCEXPROOTKEY%\8.0\Setup\VC
SET MSVC9KEY=%MSVCROOTKEY%\9.0\Setup\VC
SET MSVC9EXPRESSKEY=%MSVCEXPROOTKEY%\9.0\Setup\VC
SET MSVC10KEY=%MSVCROOTKEY%\10.0\Setup\VC
SET MSVC10EXPRESSKEY=%MSVCEXPROOTKEY%\10.0\Setup\VC
SET MSVC11KEY=%MSVCROOTKEY%\11.0\Setup\VC
SET MSVC11EXPRESSKEY=%MSVCEXPROOTKEY%\11.0\Setup\VC
SET MSVC12KEY=%MSVCROOTKEY%\12.0\Setup\VC
SET MSVC12EXPRESSKEY=%MSVCEXPROOTKEY%\12.0\Setup\VC

REM First see if we can find MSVC, then set the variable
REM NOTE: delims=<tab><space>
REM NOTE: run the initial REQ QUERY outside of the if() to set ERRORLEVEL correctly

REG QUERY "%MSVC71KEY%" /v ProductDir >nul 2>nul
if "%VC71DIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC71KEY%" /v ProductDir') DO SET VC71DIR=%%B
  )
)

REG QUERY "%MSVC8KEY%" /v ProductDir >nul 2>nul
if "%VC8DIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC8KEY%" /v ProductDir') DO SET VC8DIR=%%B
  )
)

REG QUERY "%MSVC8EXPRESSKEY%" /v ProductDir >nul 2>nul
if "%VC8EXPRESSDIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC8EXPRESSKEY%" /v ProductDir') DO SET VC8EXPRESSDIR=%%B
  )
)

REG QUERY "%MSVC9KEY%" /v ProductDir >nul 2>nul
if "%VC9DIR%"=="" (
  REM Newer SDKs (6.1, 7.0) install the VC9 compilers and set this key,
  REM but they're functionally equivalent to the VC9 Express compilers.
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC9KEY%" /v ProductDir') DO SET TEMPVC9DIR=%%B
  )
)

REM We'll double-check for a VC9 Pro install here per the comment above.
REG QUERY "%MSVCROOTKEY%\9.0\InstalledProducts\Microsoft Visual C++" >nul 2>nul
if NOT "%TEMPVC9DIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    SET "VC9DIR=%TEMPVC9DIR%"
  ) ELSE (
    SET "VC9EXPRESSDIR=%TEMPVC9DIR%"
  )
)

REG QUERY "%MSVC9EXPRESSKEY%" /v ProductDir >nul 2>nul
if "%VC9EXPRESSDIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC9EXPRESSKEY%" /v ProductDir') DO SET VC9EXPRESSDIR=%%B
  )
)

REG QUERY "%MSVC10KEY%" /v ProductDir >nul 2>nul
if "%VC10DIR%"=="" (
  REM Newer SDKs (7.1) install the VC10 compilers and set this key,
  REM but they're functionally equivalent to the VC10 Express compilers.
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC10KEY%" /v ProductDir') DO SET TEMPVC10DIR=%%B
  )
)

REM We'll double-check for a VC10 Pro install here per the comment above.
REG QUERY "%MSVCROOTKEY%\10.0\InstalledProducts\Microsoft Visual C++" >nul 2>nul
if NOT "%TEMPVC10DIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    SET "VC10DIR=%TEMPVC10DIR%"
  ) ELSE (
    SET "VC10EXPRESSDIR=%TEMPVC10DIR%"
  )
)

REG QUERY "%MSVC10EXPRESSKEY%" /v ProductDir >nul 2>nul
if "%VC10EXPRESSDIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC10EXPRESSKEY%" /v ProductDir') DO SET VC10EXPRESSDIR=%%B
  )
)

REG QUERY "%MSVC12KEY%" /v ProductDir >nul 2>nul
if "%VC12DIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC12KEY%" /v ProductDir') DO SET VC12DIR=%%B
  )
)

REG QUERY "%MSVC12EXPRESSKEY%" /v ProductDir >nul 2>nul
if "%VC12EXPRESSDIR%"=="" (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%MSVC12EXPRESSKEY%" /v ProductDir') DO SET VC12EXPRESSDIR=%%B
  )
)

REM Look for Installed SDKs:
SET SDKROOTKEY=HKLM\SOFTWARE\Microsoft\MicrosoftSDK\InstalledSDKs
SET SDK2003SP1KEY=%SDKROOTKEY%\8F9E5EF3-A9A5-491B-A889-C58EFFECE8B3
SET SDK2003SP2KEY=%SDKROOTKEY%\D2FF9F89-8AA2-4373-8A31-C838BF4DBBE1
SET SDK6KEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v6.0
REM 6.0A comes with Visual C++ 2008. If you have 6.0 installed, even
REM the express edition will update you to 6.0A
SET SDK6AKEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v6.0A
SET SDK61KEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v6.1
SET SDK7KEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0
SET SDK7AKEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.0A
SET SDK71KEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v7.1
SET SDK80KEY=HKLM\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v8.0

REM Just a base value to compare against
SET SDKDIR=
SET SDKVER=0
SET SDKMINORVER=0

REM Support a maximum version of the Windows SDK to use, to support older
REM branches and older compilers.  (Note that this is unrelated to the configure
REM option on which version of Windows to support.)
IF NOT DEFINED MOZ_MAXWINSDK (
  REM Maximum WinSDK version to use; 2 digits for major, 2 for minor, 2 for revision
  REM Revivsion is A = 01, B = 02, etc.
  SET MOZ_MAXWINSDK=999999
)

REG QUERY "%SDK80KEY%" /v InstallationFolder >nul 2>nul
if "%SDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 80000 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK80KEY%" /v InstallationFolder') DO SET SDKDIR=%%B
	SET SDKVER=8
	SET SDKMINORVER=0
  )
)

REG QUERY "%SDK71KEY%" /v InstallationFolder >nul 2>nul
if "%SDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 70100 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK71KEY%" /v InstallationFolder') DO SET SDKDIR=%%B
    SET SDKVER=7
    SET SDKMINORVER=1
  )
)

REG QUERY "%SDK7AKEY%" /v InstallationFolder >nul 2>nul
if "%SDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 70001 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK7AKEY%" /v InstallationFolder') DO SET SDKDIR=%%B
    SET SDKVER=7
    SET SDKMINORVER=0A
  )
)

REG QUERY "%SDK7KEY%" /v InstallationFolder >nul 2>nul
if "%SDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 70000 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK7KEY%" /v InstallationFolder') DO SET SDKDIR=%%B
    SET SDKVER=7
  )
)

REG QUERY "%SDK61KEY%" /v InstallationFolder >nul 2>nul
if "%SDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 60100 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK61KEY%" /v InstallationFolder') DO SET SDKDIR=%%B
    SET SDKVER=6
    SET SDKMINORVER=1
  )
)

REG QUERY "%SDK6AKEY%" /v InstallationFolder >nul 2>nul
if "%SDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 60001 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK6AKEY%" /v InstallationFolder') DO SET SDKDIR=%%B
    SET SDKVER=6
    REM insane, I know!
    SET SDKMINORVER=0A
  )
)

REG QUERY "%SDK6KEY%" /v InstallationFolder >nul 2>nul
if "%SDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 60000 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK6KEY%" /v InstallationFolder') DO SET SDKDIR=%%B
    SET SDKVER=6
  )
)

REM The Vista SDK (6.0 and 6.0A) ships with a VC8 Express install

REG QUERY "%SDK6KEY%\WinSDKCompiler" /v InstallationFolder >nul 2>nul
IF %ERRORLEVEL% EQU 0 (
  FOR /F "tokens=2*" %%A IN ('REG QUERY "%SDK6KEY%\WinSDKCompiler" /v InstallationFolder') DO SET VC8EXPRESSDIR=%%BVC\
)

REG QUERY "%SDK2003SP2KEY%" /v "Install Dir" >nul 2>nul
if "%PSDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 50000 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=3*" %%A IN ('REG QUERY "%SDK2003SP2KEY%" /v "Install Dir"') DO SET PSDKDIR=%%B
    REM arbitrary, but works for me
    SET PSDKVER=5
  )
)

REG QUERY "%SDK2003SP1KEY%" /v "Install Dir" >nul 2>nul
if "%PSDKDIR%"=="" IF %MOZ_MAXWINSDK% GEQ 40000 (
  IF %ERRORLEVEL% EQU 0 (
    FOR /F "tokens=3*" %%A IN ('REG QUERY "%SDK2003SP1KEY%" /v "Install Dir"') DO SET PSDKDIR=%%B
    SET PSDKVER=4
  )
)

if defined VC71DIR ECHO Visual C++ 7.1 directory: %VC71DIR%
if defined VC8DIR ECHO Visual C++ 8 directory: %VC8DIR%
if defined VC8EXPRESSDIR ECHO Visual C++ 8 Express directory: %VC8EXPRESSDIR%
if defined VC9DIR ECHO Visual C++ 9 directory: %VC9DIR%
if defined VC9EXPRESSDIR ECHO Visual C++ 9 Express directory: %VC9EXPRESSDIR%
if defined VC10DIR ECHO Visual C++ 10 directory: %VC10DIR%
if defined VC10EXPRESSDIR ECHO Visual C++ 10 Express directory: %VC10EXPRESSDIR%
if defined VC11DIR ECHO Visual C++ 11 directory: %VC11DIR%
if defined VC11EXPRESSDIR ECHO Visual C++ 11 Express directory: %VC11EXPRESSDIR%
if defined VC12DIR ECHO Visual C++ 12 directory: %VC12DIR%
if defined VC12EXPRESSDIR ECHO Visual C++ 12 Express directory: %VC12EXPRESSDIR%

setlocal enableextensions enabledelayedexpansion

if "!SDKDIR!"=="" (
    SET SDKDIR=!PSDKDIR!
    SET SDKVER=%PSDKVER%
) else (
    ECHO Windows SDK directory: !SDKDIR!
    ECHO Windows SDK version: %SDKVER%.%SDKMINORVER%
)
if not "!PSDKDIR!"=="" (
    ECHO Platform SDK directory: !PSDKDIR!
    ECHO Platform SDK version: %PSDKVER%
)

setlocal disableextensions enabledelayedexpansion
