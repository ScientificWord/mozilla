# Microsoft Developer Studio Project File - Name="npapi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=npapi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "npapi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "npapi.mak" CFG="npapi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "npapi - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "npapi - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "npapi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPAPI_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\include" /I "..\..\..\..\..\dist\include" /I "..\..\..\..\..\dist\include\java" /I "..\..\..\..\..\dist\include\nspr" /I "..\..\..\..\..\dist\include\plugin" /D "XP_WIN" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPAPI_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /dll /machine:I386 /out:"Y:\mb\microbrowser\bin\Win32\Debug\Plugins\npapi.dll"

!ELSEIF  "$(CFG)" == "npapi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPAPI_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\include" /I "..\..\..\..\..\dist\include\plugin" /I "..\..\..\..\..\dist\include\java" /I "..\..\..\..\..\dist\include\nspr" /D "XP_WIN" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "NPAPI_EXPORTS" /FR /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib /nologo /dll /debug /machine:I386 /out:"Y:\mb\microbrowser\bin\Win32\Debug\Plugins\npapi.dll" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "npapi - Win32 Release"
# Name "npapi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\dlgauto.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgman.cpp
# End Source File
# Begin Source File

SOURCE=.\dlgtstr.cpp
# End Source File
# Begin Source File

SOURCE=.\guihlp.cpp
# End Source File
# Begin Source File

SOURCE=.\loadstatus.cpp
# End Source File
# Begin Source File

SOURCE=..\common\log.cpp
# End Source File
# Begin Source File

SOURCE=..\common\logfile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\logger.cpp
# End Source File
# Begin Source File

SOURCE=..\common\loghlp.cpp
# End Source File
# Begin Source File

SOURCE=..\common\np_entry.cpp
# End Source File
# Begin Source File

SOURCE=.\npapi.def
# End Source File
# Begin Source File

SOURCE=.\npapi.rc
# End Source File
# Begin Source File

SOURCE=..\common\npn_gate.cpp
# End Source File
# Begin Source File

SOURCE=..\common\npp_gate.cpp
# End Source File
# Begin Source File

SOURCE=..\common\plugbase.cpp
# End Source File
# Begin Source File

SOURCE=.\plugin.cpp
# End Source File
# Begin Source File

SOURCE=..\common\pplib.cpp
# End Source File
# Begin Source File

SOURCE=..\common\profile.cpp
# End Source File
# Begin Source File

SOURCE=..\common\script.cpp
# End Source File
# Begin Source File

SOURCE=..\common\scripter.cpp
# End Source File
# Begin Source File

SOURCE=..\common\scrpthlp.cpp
# End Source File
# Begin Source File

SOURCE=..\common\strconv.cpp
# End Source File
# Begin Source File

SOURCE=.\winentry.cpp
# End Source File
# Begin Source File

SOURCE=.\winutils.cpp
# End Source File
# Begin Source File

SOURCE=..\common\xp.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\include\action.h
# End Source File
# Begin Source File

SOURCE=..\include\comstrs.h
# End Source File
# Begin Source File

SOURCE=.\guihlp.h
# End Source File
# Begin Source File

SOURCE=.\guiprefs.h
# End Source File
# Begin Source File

SOURCE=..\include\helpers.h
# End Source File
# Begin Source File

SOURCE=..\include\loadstatus.h
# End Source File
# Begin Source File

SOURCE=..\include\log.h
# End Source File
# Begin Source File

SOURCE=..\include\logfile.h
# End Source File
# Begin Source File

SOURCE=..\include\logger.h
# End Source File
# Begin Source File

SOURCE=..\include\loghlp.h
# End Source File
# Begin Source File

SOURCE=..\include\plugbase.h
# End Source File
# Begin Source File

SOURCE=.\plugin.h
# End Source File
# Begin Source File

SOURCE=..\include\profile.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=..\include\script.h
# End Source File
# Begin Source File

SOURCE=..\include\scripter.h
# End Source File
# Begin Source File

SOURCE=..\include\scrpthlp.h
# End Source File
# Begin Source File

SOURCE=..\include\strconv.h
# End Source File
# Begin Source File

SOURCE=.\winutils.h
# End Source File
# Begin Source File

SOURCE=..\include\xp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\icon.ico
# End Source File
# End Group
# End Target
# End Project
