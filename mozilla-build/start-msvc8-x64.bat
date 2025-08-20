@echo off

SETLOCAL
SET MOZ_MSVCVERSION=8
SET MOZBUILDDIR=%~dp0
SET MOZILLABUILD=%MOZBUILDDIR%

echo "Mozilla tools directory: %MOZBUILDDIR%"

REM Get MSVC paths
call "%MOZBUILDDIR%guess-msvc.bat"

REM Use the "new" moztools-static
set "MOZ_TOOLS=%MOZBUILDDIR%moztools-x64"

rem append moztools to PATH
SET "PATH=%PATH%;%MOZ_TOOLS%\bin"

if "%VC8DIR%"=="" (
    if "%VC8EXPRESSDIR%"=="" (
        ECHO "Microsoft Visual C++ version 8 was not found. Exiting."
        pause
        EXIT /B 1
    )

    if "%SDKDIR%"=="" (
        ECHO "Microsoft Platform SDK was not found. Exiting."
        pause
        EXIT /B 1
    )

    rem Prepend MSVC paths
    call "%VC8EXPRESSDIR%\Bin\amd64\vcvarsamd64.bat" 2>nul
    if "%DevEnvDir%"=="" (
      rem Might be using a compiler that shipped with an SDK, so manually set paths
      SET "PATH=%VC8EXPRESSDIR%\Bin\x64;%VC8EXPRESSDIR%\Bin;%PATH%"
      SET "INCLUDE=%VC8EXPRESSDIR%\Include;%VC8EXPRESSDIR%\Include\Sys;%INCLUDE%"
      SET "LIB=%VC8EXPRESSDIR%\Lib\x64;%VC8EXPRESSDIR%\Lib;%LIB%"
    )
    SET USESDK=1
    rem Don't set SDK paths in this block, because blocks are early-evaluated.

    rem Fix problem with VC++Express Edition
    if "%SDKVER%" GEQ "6" (
        rem SDK Ver.6.0 (Windows Vista SDK) and newer
        rem do not contain ATL header files.
        rem We need to use the Platform SDK's ATL header files.
        SET USEPSDKATL=1
    )
    rem SDK ver.6.0 does not contain OleAcc.idl
    rem We need to use the Platform SDK's OleAcc.idl
    if "%SDKVER%" == "6" (
        if "%SDKMINORVER%" == "0" (
          SET USEPSDKIDL=1
        )
    )
) else (
    rem Prepend MSVC paths
    call "%VC8DIR%\Bin\amd64\vcvarsamd64.bat"

    rem If the SDK is Win2k3SP2 or higher, we want to use it
    if "%SDKVER%" GEQ "5" (
      SET USESDK=1
    ) else (
        ECHO Using VC 2005 built-in SDK
    )
)
if "%USESDK%"=="1" (
    rem Prepend SDK paths - Don't use the SDK SetEnv.cmd because it pulls in
    rem random VC paths which we don't want.
    rem Add the atlthunk compat library to the end of our LIB
    set "PATH=%SDKDIR%\bin\x64;%SDKDIR%\bin;%PATH%"
    set "LIB=%SDKDIR%\lib\x64;%SDKDIR%\lib;%LIB%;%MOZBUILDDIR%atlthunk_compat"

    if "%USEPSDKATL%"=="1" (
        if "%USEPSDKIDL%"=="1" (
            set "INCLUDE=%SDKDIR%\include;%PSDKDIR%\include\atl;%PSDKDIR%\include;%INCLUDE%"
        ) else (
            set "INCLUDE=%SDKDIR%\include;%PSDKDIR%\include\atl;%INCLUDE%"
        )
    ) else (
        if "%USEPSDKIDL%"=="1" (
            set "INCLUDE=%SDKDIR%\include;%SDKDIR%\include\atl;%PSDKDIR%\include;%INCLUDE%"
        ) else (
            set "INCLUDE=%SDKDIR%\include;%SDKDIR%\include\atl;%INCLUDE%"
        )
    )
)

cd "%USERPROFILE%"

"%MOZILLABUILD%\msys\bin\bash" --login -i
