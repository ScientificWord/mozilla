; basic script template for NSIS installers
;
; Written by Philip Chu
; Copyright (c) 2004-2005 Technicat, LLC
;
; This software is provided 'as-is', without any express or implied warranty.
; In no event will the authors be held liable for any damages arising from the use of this software.
 
; Permission is granted to anyone to use this software for any purpose,
; including commercial applications, and to alter it ; and redistribute
; it freely, subject to the following restrictions:
 
;    1. The origin of this software must not be misrepresented; you must not claim that
;       you wrote the original software. If you use this software in a product, an
;       acknowledgment in the product documentation would be appreciated but is not required.
 
;    2. Altered source versions must be plainly marked as such, and must not be
;       misrepresented as being the original software.
 
;    3. This notice may not be removed or altered from any source distribution.
 
; The following 3 to be defined on the command line
; !define PRINCEBASE c:\prince2
; !define BUILD release
; !define setup "SWPSetup.exe"

!addplugindir "C:\mozilla-build\nsis-2.46u\Plugins"
!define princedir "${PRINCEBASE}\${BUILD}\SWP"
 
; change this to wherever the files to be packaged reside
!define srcdir ${princedir}
!define srctree "${PRINCEBASE}\mozilla"
 
!define company "MacKichan Software"
 
!define prodname "Scientific WorkPlace 6.0"
!define exec "SWP.exe"
!define filetype "SciWordDocument"
!define INSTDIR "$PROGRAMFILES\SWP" 
; optional stuff
 
; text file to open in notepad after installation
; !define notefile "README.txt"
 
; license text file
!define licensefile "license.txt"
 
; icons must be Microsoft .ICO files
!define icon "prince.ico"
 
; installer background screen
!define screenimage "${srctree}\prince\base\content\SWP.png"
 
; file containing list of file-installation commands
!define files "${srctree}\prince\install\files.nsi"
 
; file containing list of file-uninstall commands
!define unfiles "${srctree}\prince\install\unfiles.nsi"
 
; registry stuff
 
!define regkey "Software\${company}\${prodname}"
!define uninstkey "Software\Microsoft\Windows\CurrentVersion\Uninstall\${prodname}"
 
!define startmenu "$SMPROGRAMS\${prodname}"
!define uninstaller "SWPUninstall.exe"
 
;--------------------------------
!include "MUI2.nsh"

XPStyle on
ShowInstDetails hide
ShowUninstDetails hide
SetCompressor /FINAL lzma
 
Name "${prodname}"
Caption "${prodname}"
 
!ifdef icon
Icon "${srctree}\prince\icons\${icon}"
!endif
 
OutFile "${PRINCEBASE}\${BUILD}\${setup}"
 
SetDateSave on
SetDatablockOptimize on
CRCCheck on
SilentInstall normal
 
InstallDir "$PROGRAMFILES\SWP"
InstallDirRegKey HKLM "${regkey}" ""
 
!ifdef licensefile
LicenseText "License"
LicenseData "${srctree}\prince\install\${licensefile}"
!endif
 
; pages
; we keep it simple - leave out selectable installation types
 
!ifdef licensefile
Page license
!endif
 
; Page components
Page directory
Page instfiles
 
UninstPage uninstConfirm
UninstPage instfiles
 
;--------------------------------
 
AutoCloseWindow false
ShowInstDetails show
 
 
!ifdef screenimage
 
; set up background image
; uses BgImage plugin
 
Function .onGUIInit
	; extract background BMP into temp plugin directory
	InitPluginsDir
;	File /oname=$PLUGINSDIR\1.png "${screenimage}"
 
;	BgImage::SetBg /NOUNLOAD /FILLSCREEN $PLUGINSDIR\1.png
	BgImage::SetBg /NOUNLOAD /FILLSCREEN ${screenimage}
	BgImage::Redraw /NOUNLOAD
FunctionEnd
 
Function .onGUIEnd
	; Destroy must not have /NOUNLOAD so NSIS will be able to unload and delete BgImage before it exits
	BgImage::Destroy
FunctionEnd
 
!endif
 
; beginning (invisible) section
Section
 
; Install and uninstall
  WriteRegStr HKLM "${regkey}" "Install_Dir" "$INSTDIR"
  ; write uninstall strings
  WriteRegStr HKLM "${uninstkey}" "DisplayName" "${prodname} (remove only)"
  WriteRegStr HKLM "${uninstkey}" "UninstallString" '"$INSTDIR\${uninstaller}"'

;  Treat .sci files as zip files
  WriteRegDWORD HKLM "SOFTWARE\Classes\SciWordDocument" "EditFlags"	 0
  WriteRegDWORD HKLM "SOFTWARE\Classes\SciWordDocument" "BrowserFlags"	 8
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument" "AlwaysShowExt"	 ""
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument" "" "ScientificWordDocument"
  WriteRegBin HKLM "SOFTWARE\Classes\SciWordDocument" "FriendlyTypeName"  \
  53006300690065006e0074006900660069006300200057006f0072006400200044006f00630075006d0065006e0074000000
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument\CLSID"	 ""	 "{E88DCCE0-B7B3-11d1-A9F0-00AA0060FA31}"
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument\DefaultIcon"  ""  "$INSTDIR\prince.ico,0"
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument\Shell"  ""  "Open"
  WriteRegDWORD HKLM "SOFTWARE\Classes\SciWordDocument\Shell\find"  "SuppressionPolicy"  80
  WriteRegExpandStr HKLM "SOFTWARE\Classes\SciWordDocument\Shell\find\command" "" "%windir%\explore.exe"
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument\Shell\Open\command" "" '${INSTDIR}\${exec} "%1"'
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument\ShellEx\ContextMenuHandlers\{b8cdcb65-b1bf-4b42-9428-1dfdb7ee92af}" "" "Compressed (zipped) Folder Menu"
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument\ShellEx\DropHandler" "" "{ed9d80b9-d157-457b-9192-0e7280313bf0}"
  WriteRegStr HKLM "SOFTWARE\Classes\SciWordDocument\ShellEx\StorageHandler" "" "{E88DCCE0-B7B3-11d1-A9F0-00AA0060FA31}"
; .sci registration
  WriteRegStr HKLM "SOFTWARE\Classes\.sci" "" "SciWordDocument"
  WriteRegStr HKLM "SOFTWARE\Classes\.sci" "Content Type" "application/x-zip-compressed"
  WriteRegStr HKLM "SOFTWARE\Classes\.sci\OpenWithProgids" "SciWordDocument" ""
  WriteRegStr HKLM "SOFTWARE\Classes\.sci\PersistentHandler" "" "{098f2470-bae0-11cd-b579-08002b30bfeb}"
  WriteRegStr HKLM "SOFTWARE\Classes\.sci\SciWordDocument\ShellNew" "FileName" "New Scientific WorkPlace 6 Document.sci"
; Places bar
;  WriteRegExpandStr HKCU "Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar" "Place0" "$DOCUMENTS\swpdocs"
;  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar" "Place1" 00000000
;  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar" "Place2" 00000005
;  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar" "Place3" 00000011
;  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar" "Place4" 00000012




!ifdef filetype
  WriteRegStr HKCR "${filetype}" "" "${prodname}"
!endif
 
  WriteRegStr HKCR "${prodname}\Shell\open\command\" "" '$INSTDIR\${exec} "%1"'
 
!ifdef icon
  WriteRegStr HKCR "${prodname}\DefaultIcon" "" "$INSTDIR\${icon}"
!endif
 
  SetOutPath $INSTDIR
 
 
; package all files, recursively, preserving attributes
; assume files are in the correct places
File /a "${srcdir}\${exec}"
 
!ifdef licensefile
File /a "${srctree}\prince\install\${licensefile}"
!endif
 
!ifdef notefile
File /a "${srcdir}\${notefile}"
!endif
 
!ifdef icon
File /a "${srctree}\prince\icons\${icon}"
!endif
 
File "/oname=$TEMPLATES\New Scientific WorkPlace 6 Document.sci" "${srcdir}\shells\Articles\Blank_LaTeX_Article.sci"
; any application-specific files
!ifdef files
!include "${files}"
!endif
 
  WriteUninstaller "${uninstaller}"
 
SectionEnd
 
; create shortcuts
Section
 
  CreateDirectory "${startmenu}"
  SetOutPath $INSTDIR ; for working directory
!ifdef icon
  CreateShortCut "${startmenu}\${prodname}.lnk" "$INSTDIR\${exec}" "" "$INSTDIR\${icon}"
!else
  CreateShortCut "${startmenu}\${prodname}.lnk" "$INSTDIR\${exec}"
!endif
 
!ifdef notefile
  CreateShortCut "${startmenu}\Release Notes.lnk "$INSTDIR\${notefile}"
!endif
 
!ifdef helpfile
  CreateShortCut "${startmenu}\Documentation.lnk "$INSTDIR\${helpfile}"
!endif
 
!ifdef website
WriteINIStr "${startmenu}\web site.url" "InternetShortcut" "URL" ${website}
 ; CreateShortCut "${startmenu}\Web Site.lnk "${website}" "URL"
!endif
 
!ifdef notefile
ExecShell "open" "$INSTDIR\${notefile}"
!endif
 
SectionEnd
 
; Uninstaller
; All section names prefixed by "Un" will be in the uninstaller
 
UninstallText "This will uninstall ${prodname}."
 
!ifdef icon
UninstallIcon "${srctree}\prince\icons\${icon}"
!endif
 
Section "Uninstall"
 
  DeleteRegKey HKLM "${uninstkey}"
  DeleteRegKey HKLM "${regkey}"
  DeleteRegKey HKLM "SOFTWARE\Classes\SciWordDocument"  
  DeleteRegKey HKLM "SOFTWARE\Classes\.sci"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Policies\ComDlg32\PlacesBar"
  Delete "${startmenu}\*.*"
  Delete "${startmenu}"
 
!ifdef licensefile
Delete "$INSTDIR\${licensefile}"
!endif
 
!ifdef notefile
Delete "$INSTDIR\${notefile}"
!endif
 
!ifdef icon
Delete "$INSTDIR\${icon}"
!endif
 
Delete "$INSTDIR\${exec}"
 
!ifdef unfiles
!include "${unfiles}"
!endif
 
SectionEnd
