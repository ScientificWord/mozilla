# ***** BEGIN LICENSE BLOCK *****
# Version: MPL 1.1/GPL 2.0/LGPL 2.1
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Original Code is the Mozilla Installer code.
#
# The Initial Developer of the Original Code is Mozilla Foundation
# Portions created by the Initial Developer are Copyright (C) 2006
# the Initial Developer. All Rights Reserved.
#
# Contributor(s):
#  Robert Strong <robert.bugzilla@gmail.com>
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 2 or later (the "GPL"), or
# the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
# in which case the provisions of the GPL or the LGPL are applicable instead
# of those above. If you wish to allow use of your version of this file only
# under the terms of either the GPL or the LGPL, and not to allow others to
# use your version of this file under the terms of the MPL, indicate your
# decision by deleting the provisions above and replace them with the notice
# and other provisions required by the GPL or the LGPL. If you do not delete
# the provisions above, a recipient may use your version of this file under
# the terms of any one of the MPL, the GPL or the LGPL.
#
# ***** END LICENSE BLOCK *****

################################################################################
# Helper defines and macros for toolkit applications

/**
 * Avoid creating macros / functions that overwrite registers (see the
 * GetLongPath macro for one way to avoid this)!
 *
 * Before using the registers exchange the passed in params and save existing
 * register values to the stack.
 *
 * Exch $R9 ; exhange the original $R9 with the top of the stack
 * Exch 1   ; exchange the top of the stack with 1 below the top of the stack
 * Exch $R8 ; exchange the original $R8 with the top of the stack
 * Exch 2   ; exchange the top of the stack with 2 below the top of the stack
 * Exch $R7 ; exchange the original $R7 with the top of the stack
 * Push $R6 ; push the original $R6 onto the top of the stack
 * Push $R5 ; push the original $R5 onto the top of the stack
 * Push $R4 ; push the original $R4 onto the top of the stack
 *
 * <do stuff>
 *
 * ; Restore the values.
 * Pop $R4  ; restore the value for $R4 from the top of the stack
 * Pop $R5  ; restore the value for $R5 from the top of the stack
 * Pop $R6  ; restore the value for $R6 from the top of the stack
 * Exch $R7 ; exchange the new $R7 value with the top of the stack
 * Exch 2   ; exchange the top of the stack with 2 below the top of the stack
 * Exch $R8 ; exchange the new $R8 value with the top of the stack
 * Exch 1   ; exchange the top of the stack with 2 below the top of the stack
 * Exch $R9 ; exchange the new $R9 value with the top of the stack
 *
 *
 * When inserting macros in common.nsh from another macro in common.nsh that
 * can be used from the uninstaller _MOZFUNC_UN will be undefined when it is
 * inserted. Use the following to redefine _MOZFUNC_UN with its original value
 * (see the RegCleanMain macro for an example).
 *
 * !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
 * !insertmacro ${_MOZFUNC_UN_TMP}FileJoin
 * !insertmacro ${_MOZFUNC_UN_TMP}LineFind
 * !insertmacro ${_MOZFUNC_UN_TMP}TextCompareNoDetails
 * !insertmacro ${_MOZFUNC_UN_TMP}TrimNewLines
 * !undef _MOZFUNC_UN
 * !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
 * !undef _MOZFUNC_UN_TMP
 */

; When including a file provided by NSIS check if its verbose macro is defined
; to prevent loading the file a second time.
!ifmacrondef TEXTFUNC_VERBOSE
  !include TextFunc.nsh
!endif

!ifmacrondef FILEFUNC_VERBOSE
  !include FileFunc.nsh
!endif

!ifmacrondef LOGICLIB_VERBOSITY
  !include LogicLib.nsh
!endif

!ifndef WINMESSAGES_INCLUDED
  !include WinMessages.nsh
!endif

!ifndef MUI_VERBOSE
  !include MUI.nsh
!endif

; When including WinVer.nsh check if ___WINVER__NSH___ is defined to prevent
; loading the file a second time. NSIS versions prior to 2.21 didn't include
; WinVer.nsh so include it with the /NOFATAL option.
!ifndef ___WINVER__NSH___
  !include /NONFATAL WinVer.nsh
!endif

; NSIS provided macros that we have overridden.
!include overrides.nsh

################################################################################
# Macros for debugging

/**
 * The following two macros assist with verifying that a macro doesn't
 * overwrite any registers.
 *
 * Usage:
 * ${debugSetRegisters}
 * <do stuff>
 * ${debugDisplayRegisters}
 */

/**
 * Sets all register values to their name to assist with verifying that a macro
 * doesn't overwrite any registers.
 */
!macro debugSetRegisters
  StrCpy $0 "$$0"
  StrCpy $1 "$$1"
  StrCpy $2 "$$2"
  StrCpy $3 "$$3"
  StrCpy $4 "$$4"
  StrCpy $5 "$$5"
  StrCpy $6 "$$6"
  StrCpy $7 "$$7"
  StrCpy $8 "$$8"
  StrCpy $9 "$$9"
  StrCpy $R0 "$$R0"
  StrCpy $R1 "$$R1"
  StrCpy $R2 "$$R2"
  StrCpy $R3 "$$R3"
  StrCpy $R4 "$$R4"
  StrCpy $R5 "$$R5"
  StrCpy $R6 "$$R6"
  StrCpy $R7 "$$R7"
  StrCpy $R8 "$$R8"
  StrCpy $R9 "$$R9"
!macroend
!define debugSetRegisters "!insertmacro debugSetRegisters"

/**
 * Displays all register values to assist with verifying that a macro doesn't
 * overwrite any registers.
 */
!macro debugDisplayRegisters
  MessageBox MB_OK \
      "Register Values:$\n\
       $$0 = $0$\n$$1 = $1$\n$$2 = $2$\n$$3 = $3$\n$$4 = $4$\n\
       $$5 = $5$\n$$6 = $6$\n$$7 = $7$\n$$8 = $8$\n$$9 = $9$\n\
       $$R0 = $R0$\n$$R1 = $R1$\n$$R2 = $R2$\n$$R3 = $R3$\n$$R4 = $R4$\n\
       $$R5 = $R5$\n$$R6 = $R6$\n$$R7 = $R7$\n$$R8 = $R8$\n$$R9 = $R9"
!macroend
!define debugDisplayRegisters "!insertmacro debugDisplayRegisters"


################################################################################
# Modern User Interface (MUI) override macros

; Modified version of the following MUI macros to support Mozilla localization.
; MUI_LANGUAGE
; MUI_LANGUAGEFILE_BEGIN
; MOZ_MUI_LANGUAGEFILE_END
; See <NSIS App Dir>/Contrib/Modern UI/System.nsh for more information
!define MUI_INSTALLOPTIONS_READ "!insertmacro MUI_INSTALLOPTIONS_READ"

!macro MOZ_MUI_LANGUAGE LANGUAGE
  !verbose push
  !verbose ${MUI_VERBOSE}
  !include "${LANGUAGE}.nsh"
  !verbose pop
!macroend

!macro MOZ_MUI_LANGUAGEFILE_BEGIN LANGUAGE
  !ifndef MUI_INSERT
    !define MUI_INSERT
    !insertmacro MUI_INSERT
  !endif
  !ifndef "MUI_LANGUAGEFILE_${LANGUAGE}_USED"
    !define "MUI_LANGUAGEFILE_${LANGUAGE}_USED"
    LoadLanguageFile "${LANGUAGE}.nlf"
  !else
    !error "Modern UI language file ${LANGUAGE} included twice!"
  !endif
!macroend

; Custom version of MUI_LANGUAGEFILE_END. The macro to add the default MUI
; strings and the macros for several strings that are part of the NSIS MUI and
; not in our locale files have been commented out.
!macro MOZ_MUI_LANGUAGEFILE_END

#  !include "${NSISDIR}\Contrib\Modern UI\Language files\Default.nsh"
  !ifdef MUI_LANGUAGEFILE_DEFAULT_USED
    !undef MUI_LANGUAGEFILE_DEFAULT_USED
    !warning "${LANGUAGE} Modern UI language file version doesn't match. Using default English texts for missing strings."
  !endif

  !insertmacro MUI_LANGUAGEFILE_DEFINE "MUI_${LANGUAGE}_LANGNAME" "MUI_LANGNAME"

  !ifndef MUI_LANGDLL_PUSHLIST
    !define MUI_LANGDLL_PUSHLIST "'${MUI_${LANGUAGE}_LANGNAME}' ${LANG_${LANGUAGE}} "
  !else
    !ifdef MUI_LANGDLL_PUSHLIST_TEMP
      !undef MUI_LANGDLL_PUSHLIST_TEMP
    !endif
    !define MUI_LANGDLL_PUSHLIST_TEMP "${MUI_LANGDLL_PUSHLIST}"
    !undef MUI_LANGDLL_PUSHLIST
    !define MUI_LANGDLL_PUSHLIST "'${MUI_${LANGUAGE}_LANGNAME}' ${LANG_${LANGUAGE}} ${MUI_LANGDLL_PUSHLIST_TEMP}"
  !endif

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE WELCOME "MUI_TEXT_WELCOME_INFO_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE WELCOME "MUI_TEXT_WELCOME_INFO_TEXT"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE LICENSE "MUI_TEXT_LICENSE_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE LICENSE "MUI_TEXT_LICENSE_SUBTITLE"
  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE LICENSE "MUI_INNERTEXT_LICENSE_TOP"

#  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE LICENSE "MUI_INNERTEXT_LICENSE_BOTTOM"

!ifdef MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE LICENSE "MUI_INNERTEXT_LICENSE_BOTTOM_CHECKBOX"
!endif

!ifdef MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE LICENSE "MUI_INNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS"
!endif

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE COMPONENTS "MUI_TEXT_COMPONENTS_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE COMPONENTS "MUI_TEXT_COMPONENTS_SUBTITLE"
  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE COMPONENTS "MUI_INNERTEXT_COMPONENTS_DESCRIPTION_TITLE"
  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE COMPONENTS "MUI_INNERTEXT_COMPONENTS_DESCRIPTION_INFO"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE DIRECTORY "MUI_TEXT_DIRECTORY_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE DIRECTORY "MUI_TEXT_DIRECTORY_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE STARTMENU "MUI_TEXT_STARTMENU_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE STARTMENU "MUI_TEXT_STARTMENU_SUBTITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE STARTMENU "MUI_INNERTEXT_STARTMENU_TOP"
#  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE STARTMENU "MUI_INNERTEXT_STARTMENU_CHECKBOX"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE INSTFILES "MUI_TEXT_INSTALLING_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE INSTFILES "MUI_TEXT_INSTALLING_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE INSTFILES "MUI_TEXT_FINISH_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE INSTFILES "MUI_TEXT_FINISH_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE INSTFILES "MUI_TEXT_ABORT_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE INSTFILES "MUI_TEXT_ABORT_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE FINISH "MUI_BUTTONTEXT_FINISH"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE FINISH "MUI_TEXT_FINISH_INFO_TITLE"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE FINISH "MUI_TEXT_FINISH_INFO_TEXT"
  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_PAGE FINISH "MUI_TEXT_FINISH_INFO_REBOOT"
  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE FINISH "MUI_TEXT_FINISH_REBOOTNOW"
  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE FINISH "MUI_TEXT_FINISH_REBOOTLATER"
#  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE FINISH "MUI_TEXT_FINISH_RUN"
#  !insertmacro MUI_LANGUAGEFILE_MULTILANGSTRING_PAGE FINISH "MUI_TEXT_FINISH_SHOWREADME"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_DEFINE MUI_ABORTWARNING "MUI_TEXT_ABORTWARNING"


  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE WELCOME "MUI_UNTEXT_WELCOME_INFO_TITLE"
  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE WELCOME "MUI_UNTEXT_WELCOME_INFO_TEXT"

  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE CONFIRM "MUI_UNTEXT_CONFIRM_TITLE"
  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE CONFIRM "MUI_UNTEXT_CONFIRM_SUBTITLE"

#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE LICENSE "MUI_UNTEXT_LICENSE_TITLE"
#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE LICENSE "MUI_UNTEXT_LICENSE_SUBTITLE"

#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE LICENSE "MUI_UNINNERTEXT_LICENSE_BOTTOM"
#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE LICENSE "MUI_UNINNERTEXT_LICENSE_BOTTOM_CHECKBOX"
#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE LICENSE "MUI_UNINNERTEXT_LICENSE_BOTTOM_RADIOBUTTONS"

#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE COMPONENTS "MUI_UNTEXT_COMPONENTS_TITLE"
#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE COMPONENTS "MUI_UNTEXT_COMPONENTS_SUBTITLE"

#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE DIRECTORY "MUI_UNTEXT_DIRECTORY_TITLE"
#  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE DIRECTORY  "MUI_UNTEXT_DIRECTORY_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE INSTFILES "MUI_UNTEXT_UNINSTALLING_TITLE"
  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE INSTFILES "MUI_UNTEXT_UNINSTALLING_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE INSTFILES "MUI_UNTEXT_FINISH_TITLE"
  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE INSTFILES "MUI_UNTEXT_FINISH_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE INSTFILES "MUI_UNTEXT_ABORT_TITLE"
  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE INSTFILES "MUI_UNTEXT_ABORT_SUBTITLE"

  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE FINISH "MUI_UNTEXT_FINISH_INFO_TITLE"
  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE FINISH "MUI_UNTEXT_FINISH_INFO_TEXT"
  !insertmacro MUI_LANGUAGEFILE_UNLANGSTRING_PAGE FINISH "MUI_UNTEXT_FINISH_INFO_REBOOT"

  !insertmacro MUI_LANGUAGEFILE_LANGSTRING_DEFINE MUI_UNABORTWARNING "MUI_UNTEXT_ABORTWARNING"

!macroend


################################################################################
# Macros for creating Install Options ini files

!macro createComponentsINI
  WriteINIStr "$PLUGINSDIR\components.ini" "Settings" NumFields "5"

  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Type   "label"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Text   "$(OPTIONAL_COMPONENTS_LABEL)"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Left   "0"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Right  "-1"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Top    "5"
  WriteINIStr "$PLUGINSDIR\components.ini" "Field 1" Bottom "15"

  ${If} ${FileExists} "$EXEDIR\optional\extensions\inspector@mozilla.org"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" Type   "checkbox"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" Text   "$(DOMI_TITLE)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" Left   "15"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" Top    "20"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" Bottom "30"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" State  "1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 2" Flags  "GROUP"
  ${EndIf}

  ${If} ${FileExists} "$EXEDIR\optional\extensions\inspector@mozilla.org"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 3" Type   "label"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 3" Text   "$(DOMI_TEXT)"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 3" Left   "30"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 3" Right  "-1"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 3" Top    "32"
    WriteINIStr "$PLUGINSDIR\components.ini" "Field 3" Bottom "52"
  ${EndIf}
!macroend

!macro createShortcutsINI
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Settings" NumFields "4"

  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 1" Type   "label"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 1" Text   "$(CREATE_ICONS_DESC)"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 1" Left   "0"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 1" Right  "-1"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 1" Top    "5"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 1" Bottom "15"

  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" Type   "checkbox"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" Text   "$(ICONS_DESKTOP)"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" Left   "15"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" Right  "-1"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" Top    "20"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" Bottom "30"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" State  "1"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 2" Flags  "GROUP"

  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 3" Type   "checkbox"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 3" Text   "$(ICONS_STARTMENU)"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 3" Left   "15"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 3" Right  "-1"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 3" Top    "40"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 3" Bottom "50"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 3" State  "1"

  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 4" Type   "checkbox"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 4" Text   "$(ICONS_QUICKLAUNCH)"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 4" Left   "15"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 4" Right  "-1"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 4" Top    "60"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 4" Bottom "70"
  WriteINIStr "$PLUGINSDIR\shortcuts.ini" "Field 4" State  "1"
!macroend

!macro createBasicCustomOptionsINI
  WriteINIStr "$PLUGINSDIR\options.ini" "Settings" NumFields "5"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Type   "label"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Text   "$(OPTIONS_SUMMARY)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Left   "0"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Top    "0"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Bottom "10"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Type   "RadioButton"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Text   "$(OPTION_STANDARD_RADIO)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Left   "15"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Top    "25"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Bottom "35"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" State  "1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Flags  "GROUP"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Type   "RadioButton"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Text   "$(OPTION_CUSTOM_RADIO)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Left   "15"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Top    "55"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Bottom "65"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" State  "0"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Type   "label"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Text   "$(OPTION_STANDARD_DESC)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Left   "30"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Top    "37"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Bottom "57"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Type   "label"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Text   "$(OPTION_CUSTOM_DESC)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Left   "30"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Top    "67"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Bottom "87"
!macroend

!macro createBasicCompleteCustomOptionsINI
  WriteINIStr "$PLUGINSDIR\options.ini" "Settings" NumFields "7"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Type   "label"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Text   "$(OPTIONS_DESC)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Left   "0"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Top    "0"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 1" Bottom "10"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Type   "RadioButton"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Text   "$(OPTION_STANDARD_RADIO)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Left   "15"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Top    "25"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Bottom "35"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" State  "1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 2" Flags  "GROUP"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Type   "RadioButton"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Text   "$(OPTION_COMPLETE_RADIO)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Left   "15"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Top    "55"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" Bottom "65"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 3" State  "0"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Type   "RadioButton"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Text   "$(OPTION_CUSTOM_RADIO)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Left   "15"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Top    "85"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" Bottom "95"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 4" State  "0"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Type   "label"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Text   "$(OPTION_STANDARD_DESC)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Left   "30"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Top    "37"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 5" Bottom "57"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 6" Type   "label"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 6" Text   "$(OPTION_COMPLETE_DESC)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 6" Left   "30"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 6" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 6" Top    "67"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 6" Bottom "87"

  WriteINIStr "$PLUGINSDIR\options.ini" "Field 7" Type   "label"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 7" Text   "$(OPTION_CUSTOM_DESC)"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 7" Left   "30"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 7" Right  "-1"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 7" Top    "97"
  WriteINIStr "$PLUGINSDIR\options.ini" "Field 7" Bottom "117"
!macroend

!macro createSummaryINI
  WriteINIStr "$PLUGINSDIR\summary.ini" "Settings" NumFields "3"

  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 1" Type   "label"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 1" Text   "$(SUMMARY_INSTALLED_TO)"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 1" Left   "0"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 1" Right  "-1"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 1" Top    "5"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 1" Bottom "15"

  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 2" Type   "text"
  ; The contents of this control must be set as follows in the pre function
  ; ${MUI_INSTALLOPTIONS_READ} $1 "summary.ini" "Field 2" "HWND"
  ; SendMessage $1 ${WM_SETTEXT} 0 "STR:$INSTDIR"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 2" state  ""
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 2" Left   "0"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 2" Right  "-1"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 2" Top    "17"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 2" Bottom "30"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 2" flags  "READONLY"

  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 3" Type   "label"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 3" Text   "$(SUMMARY_CLICK)"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 3" Left   "0"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 3" Right  "-1"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 3" Top    "130"
  WriteINIStr "$PLUGINSDIR\summary.ini" "Field 3" Bottom "150"

  ${If} "$TmpVal" == "true"
    WriteINIStr "$PLUGINSDIR\summary.ini" "Field 4" Type   "label"
    WriteINIStr "$PLUGINSDIR\summary.ini" "Field 4" Text   "$(SUMMARY_REBOOT_REQUIRED_INSTALL)"
    WriteINIStr "$PLUGINSDIR\summary.ini" "Field 4" Left   "0"
    WriteINIStr "$PLUGINSDIR\summary.ini" "Field 4" Right  "-1"
    WriteINIStr "$PLUGINSDIR\summary.ini" "Field 4" Top    "35"
    WriteINIStr "$PLUGINSDIR\summary.ini" "Field 4" Bottom "45"

    WriteINIStr "$PLUGINSDIR\summary.ini" "Settings" NumFields "4"
  ${EndIf}

!macroend

!macro un.createUnConfirmINI
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Settings" NumFields "5"

  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 1" Type   "label"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 1" Text   "$(UN_CONFIRM_UNINSTALLED_FROM)"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 1" Left   "0"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 1" Right  "-1"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 1" Top    "5"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 1" Bottom "15"

  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 2" Type   "text"
  ; The contents of this control must be set as follows in the pre function
  ; ${MUI_INSTALLOPTIONS_READ} $1 "unconfirm.ini" "Field 2" "HWND"
  ; SendMessage $1 ${WM_SETTEXT} 0 "STR:$INSTDIR"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 2" State  ""
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 2" Left   "0"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 2" Right  "-1"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 2" Top    "17"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 2" Bottom "30"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 2" flags  "READONLY"

  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Type   "checkbox"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Text   "$(UN_REMOVE_PROFILES)"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Left   "0"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Right  "-1"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Top    "40"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Bottom "50"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" State  "0"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" flags  "NOTIFY"

  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" Type   "text"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" State   "$(UN_REMOVE_PROFILES_DESC)"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" Left   "0"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" Right  "-1"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" Top    "52"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" Bottom "120"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" flags  "MULTILINE|READONLY"

  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 5" Type   "label"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 5" Text   "$(UN_CONFIRM_CLICK)"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 5" Left   "0"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 5" Right  "-1"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 5" Top    "130"
  WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 5" Bottom "150"

  ${If} "$TmpVal" == "true"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 6" Type   "label"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 6" Text   "$(SUMMARY_REBOOT_REQUIRED_UNINSTALL)"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 6" Left   "0"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 6" Right  "-1"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 6" Top    "35"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 6" Bottom "45"

    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Settings" NumFields "6"

    ; To insert this control reset Top / Bottom for controls below this one
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Top    "55"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 3" Bottom "65"
    WriteINIStr "$PLUGINSDIR\unconfirm.ini" "Field 4" Top    "67"
  ${EndIf}
!macroend

/**
 * DEPRECATED - use GetParent instead.
 */
!macro GetParentDir
  Exch $R0
  Push $R1
  Push $R2
  Push $R3
  StrLen $R3 $R0
  ${DoWhile} 1 > 0
    IntOp $R1 $R1 - 1
    ${If} $R1 <= -$R3
      ${Break}
    ${EndIf}
    StrCpy $R2 $R0 1 $R1
    ${If} $R2 == "\"
      ${Break}
    ${EndIf}
  ${Loop}

  StrCpy $R0 $R0 $R1
  Pop $R3
  Pop $R2
  Pop $R1
  Exch $R0
!macroend
!define GetParentDir "!insertmacro GetParentDir"

/**
 * DEPRECATED - use GetPathFromString and RemoveQuotesFromPath
 *
 * Removes quotes and trailing path separator from registry string paths.
 * @param   $R0
 *          Contains the registry string
 * @return  Modified string at the top of the stack.
 */
!macro GetPathFromRegStr
  Exch $R0
  Push $R8
  Push $R9

  StrCpy $R9 "$R0" "" -1
  StrCmp $R9 '"' +2 0
  StrCmp $R9 "'" 0 +2
  StrCpy $R0 "$R0" -1

  StrCpy $R9 "$R0" 1
  StrCmp $R9 '"' +2 0
  StrCmp $R9 "'" 0 +2
  StrCpy $R0 "$R0" "" 1

  StrCpy $R9 "$R0" "" -1
  StrCmp $R9 "\" 0 +2
  StrCpy $R0 "$R0" -1

  ClearErrors
  GetFullPathName $R8 $R0
  IfErrors +2 0
  StrCpy $R0 $R8

  ClearErrors
  Pop $R9
  Pop $R8
  Exch $R0
!macroend
!define GetPathFromRegStr "!insertmacro GetPathFromRegStr"


################################################################################
# Macros for handling files in use

/**
 * Checks for files in use.
 *
 * Example usage:
 *
 *  ; The first string to be pushed onto the stack MUST be "end" to indicate
 *  ; that there are no more relative file paths to check.
 *  Push "end"
 *  Push "chrome\toolkit.jar"
 *  Push "freebl3.dll"
 *  ; The last file pushed should be the app's main exe so if it is in use this
 *  ; macro will return after the first check.
 *  Push "${FileMainEXE}"
 *  ${CheckForFilesInUse} $R9
 *
 * !IMPORTANT - this macro uses the $R7, $R8, and $R9 registers and makes no
 *              attempt to restore their original values.
 *
 * @return  _RESULT
 *          false if all of the files popped from the stack are not in use.
 *          True if any of the files popped from the stack are in use.
 * $R7 = Temporary backup directory where the files will be copied to.
 * $R8 = value popped from the stack. This will either be a relative file path
 *       from the $INSTDIR or "end" to indicate that there are no additional
 *       files to check.
 * $R9 = _RESULT
 */
!macro CheckForFilesInUse

  !ifndef ${_MOZFUNC_UN}CheckForFilesInUse
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}CheckForFilesInUse "!insertmacro ${_MOZFUNC_UN}CheckForFilesInUseCall"

    Function ${_MOZFUNC_UN}CheckForFilesInUse
      ; Create a temporary backup directory.
      GetTempFileName $R7 "$INSTDIR"
      Delete "$R7"
      SetOutPath "$R7"
      StrCpy $R9 "false"

      start:
      Pop $R8
      StrCmp "$R8" "end" end +1
      IfFileExists "$INSTDIR\$R8" +1 start

      ClearErrors
      CopyFiles /SILENT "$INSTDIR\$R8" "$R7\$R8"
      IfErrors +4 +1
      Delete "$INSTDIR\$R8"
      IfErrors +1 start
      Delete "$R7\$R8"

      StrCpy $R9 "true"

      end:
      SetOutPath "$INSTDIR"
      CopyFiles /SILENT "$R7\*" "$INSTDIR\"
      RmDir /r "$R7"
      ClearErrors

      Push $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CheckForFilesInUseCall _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call CheckForFilesInUse
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.CheckForFilesInUseCall _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.CheckForFilesInUse
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.CheckForFilesInUse
  !ifndef un.CheckForFilesInUse
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro CheckForFilesInUse

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * The macros below will automatically prepend un. to the function names when
 * they are defined (e.g. !define un.RegCleanMain).
 */
!verbose push
!verbose 3
!ifndef _MOZFUNC_VERBOSE
  !define _MOZFUNC_VERBOSE 3
!endif
!verbose ${_MOZFUNC_VERBOSE}
!define MOZFUNC_VERBOSE "!insertmacro MOZFUNC_VERBOSE"
!define _MOZFUNC_UN
!define _MOZFUNC_S
!verbose pop

!macro MOZFUNC_VERBOSE _VERBOSE
  !verbose push
  !verbose 3
  !undef _MOZFUNC_VERBOSE
  !define _MOZFUNC_VERBOSE ${_VERBOSE}
  !verbose pop
!macroend

/**
 * Displays a MessageBox and then calls abort to prevent continuing to the
 * next page when the specified Window Class is found.
 *
 * @param   _WINDOW_CLASS
 *          The Window Class to search for with FindWindow.
 * @param   _MSG
 *          The message text to display in the message box.
 *
 * $R7 = return value from FindWindow
 * $R8 = _WINDOW_CLASS
 * $R9 = _MSG
 */
!macro ManualCloseAppPrompt

  !ifndef ${_MOZFUNC_UN}ManualCloseAppPrompt
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}ManualCloseAppPrompt "!insertmacro ${_MOZFUNC_UN}ManualCloseAppPromptCall"

    Function ${_MOZFUNC_UN}ManualCloseAppPrompt
      Exch $R9
      Exch 1
      Exch $R8
      Push $R7

      FindWindow $R7 "$R8"
      IntCmp $R7 0 +3 +1 +1
      MessageBox MB_OK|MB_ICONQUESTION "$R9"
      Abort

      Pop $R7
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro ManualCloseAppPromptCall _WINDOW_CLASS _MSG
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_WINDOW_CLASS}"
  Push "${_MSG}"
  Call ManualCloseAppPrompt
  !verbose pop
!macroend

!macro un.ManualCloseAppPromptCall _WINDOW_CLASS _MSG
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_WINDOW_CLASS}"
  Push "${_MSG}"
  Call un.ManualCloseAppPrompt
  !verbose pop
!macroend

!macro un.ManualCloseAppPrompt
  !ifndef un.ManualCloseAppPrompt
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro ManualCloseAppPrompt

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Posts WM_QUIT to the application's message window which is found using the
 * message window's class. This macro uses the nsProcess plugin available
 * from http://nsis.sourceforge.net/NsProcess_plugin
 *
 * @param   _MSG
 *          The message text to display in the message box.
 * @param   _PROMPT
 *          If false don't prompt the user and automatically exit the
 *          application if it is running.
 *
 * $R6 = return value for nsProcess::_FindProcess and nsProcess::_KillProcess
 * $R7 = return value from FindWindow
 * $R8 = _PROMPT
 * $R9 = _MSG
 */
!macro CloseApp

  !ifndef ${_MOZFUNC_UN}CloseApp
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}CloseApp "!insertmacro ${_MOZFUNC_UN}CloseAppCall"

    Function ${_MOZFUNC_UN}CloseApp
      Exch $R9
      Exch 1
      Exch $R8
      Push $R7
      Push $R6

      loop:
      Push $R6
      nsProcess::_FindProcess /NOUNLOAD "${FileMainEXE}"
      Pop $R6
      StrCmp $R6 0 +1 end

      StrCmp $R8 "false" +2 +1
      MessageBox MB_OKCANCEL|MB_ICONQUESTION "$R9" IDCANCEL exit 0

      FindWindow $R7 "${WindowClass}"
      IntCmp $R7 0 +4 +1 +1
      System::Call 'user32::PostMessage(i r17, i ${WM_QUIT}, i 0, i 0)'
      # The amount of time to wait for the app to shutdown before prompting again
      Sleep 5000

      Push $R6
      nsProcess::_FindProcess /NOUNLOAD "${FileMainEXE}"
      Pop $R6
      StrCmp $R6 0 +1 end
      Push $R6
      nsProcess::_KillProcess /NOUNLOAD "${FileMainEXE}"
      Pop $R6
      Sleep 2000

      Goto loop

      exit:
      nsProcess::_Unload
      Quit

      end:
      nsProcess::_Unload

      Pop $R6
      Pop $R7
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CloseAppCall _MSG _PROMPT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_MSG}"
  Push "${_PROMPT}"
  Call CloseApp
  !verbose pop
!macroend

!macro un.CloseApp
  !ifndef un.CloseApp
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro CloseApp

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

!macro un.CloseAppCall _MSG _PROMPT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_MSG}"
  Push "${_PROMPT}"
  Call un.CloseApp
  !verbose pop
!macroend


################################################################################
# Macros for working with the registry

/**
 * Writes a registry string using SHCTX and the supplied params and logs the
 * action to the install log and the uninstall log if _LOG_UNINSTALL equals 1.
 *
 * Define NO_LOG to prevent all logging when calling this from the uninstaller.
 *
 * @param   _ROOT
 *          The registry key root as defined by NSIS (e.g. HKLM, HKCU, etc.).
 *          This will only be used for logging.
 * @param   _KEY
 *          The subkey in relation to the key root.
 * @param   _NAME
 *          The key value name to write to.
 * @param   _STR
 *          The string to write to the key value name.
 * @param   _LOG_UNINSTALL
 *          0 = don't add to uninstall log, 1 = add to uninstall log.
 *
 * $R5 = _ROOT
 * $R6 = _KEY
 * $R7 = _NAME
 * $R8 = _STR
 * $R9 = _LOG_UNINSTALL
 */
!macro WriteRegStr2

  !ifndef ${_MOZFUNC_UN}WriteRegStr2
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}WriteRegStr2 "!insertmacro ${_MOZFUNC_UN}WriteRegStr2Call"

    Function ${_MOZFUNC_UN}WriteRegStr2
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Exch 3
      Exch $R6
      Exch 4
      Exch $R5

      ClearErrors
      WriteRegStr SHCTX "$R6" "$R7" "$R8"

      !ifndef NO_LOG
        IfErrors 0 +3
        FileWrite $fhInstallLog "  ** ERROR Adding Registry String: $R5 | $R6 | $R7 | $R8 **$\r$\n"
        GoTo +4
        StrCmp "$R9" "1" +1 +2
        FileWrite $fhUninstallLog "RegVal: $R5 | $R6 | $R7$\r$\n"
        FileWrite $fhInstallLog "  Added Registry String: $R5 | $R6 | $R7 | $R8$\r$\n"
      !endif

      Exch $R5
      Exch 4
      Exch $R6
      Exch 3
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro WriteRegStr2Call _ROOT _KEY _NAME _STR _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_NAME}"
  Push "${_STR}"
  Push "${_LOG_UNINSTALL}"
  Call WriteRegStr2
  !verbose pop
!macroend

!macro un.WriteRegStr2Call _ROOT _KEY _NAME _STR _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_NAME}"
  Push "${_STR}"
  Push "${_LOG_UNINSTALL}"
  Call un.WriteRegStr2
  !verbose pop
!macroend

!macro un.WriteRegStr2
  !ifndef un.WriteRegStr2
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro WriteRegStr2

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Writes a registry dword using SHCTX and the supplied params and logs the
 * action to the install log and the uninstall log if _LOG_UNINSTALL equals 1.
 *
 * Define NO_LOG to prevent all logging when calling this from the uninstaller.
 *
 * @param   _ROOT
 *          The registry key root as defined by NSIS (e.g. HKLM, HKCU, etc.).
 *          This will only be used for logging.
 * @param   _KEY
 *          The subkey in relation to the key root.
 * @param   _NAME
 *          The key value name to write to.
 * @param   _DWORD
 *          The dword to write to the key value name.
 * @param   _LOG_UNINSTALL
 *          0 = don't add to uninstall log, 1 = add to uninstall log.
 *
 * $R5 = _ROOT
 * $R6 = _KEY
 * $R7 = _NAME
 * $R8 = _DWORD
 * $R9 = _LOG_UNINSTALL
 */
!macro WriteRegDWORD2

  !ifndef ${_MOZFUNC_UN}WriteRegDWORD2
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}WriteRegDWORD2 "!insertmacro ${_MOZFUNC_UN}WriteRegDWORD2Call"

    Function ${_MOZFUNC_UN}WriteRegDWORD2
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Exch 3
      Exch $R6
      Exch 4
      Exch $R5

      ClearErrors
      WriteRegDWORD SHCTX "$R6" "$R7" "$R8"

      !ifndef NO_LOG
        IfErrors 0 +3
        FileWrite $fhInstallLog "  ** ERROR Adding Registry DWord: $R5 | $R6 | $R7 | $R8 **$\r$\n"
        GoTo +4
        StrCmp "$R9" "1" +1 +2
        FileWrite $fhUninstallLog "RegVal: $R5 | $R6 | $R7$\r$\n"
        FileWrite $fhInstallLog "  Added Registry DWord: $R5 | $R6 | $R7 | $R8$\r$\n"
      !endif

      Exch $R5
      Exch 4
      Exch $R6
      Exch 3
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro WriteRegDWORD2Call _ROOT _KEY _NAME _DWORD _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_NAME}"
  Push "${_DWORD}"
  Push "${_LOG_UNINSTALL}"
  Call WriteRegDWORD2
  !verbose pop
!macroend

!macro un.WriteRegDWORD2Call _ROOT _KEY _NAME _DWORD _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_NAME}"
  Push "${_DWORD}"
  Push "${_LOG_UNINSTALL}"
  Call un.WriteRegDWORD2
  !verbose pop
!macroend

!macro un.WriteRegDWORD2
  !ifndef un.WriteRegDWORD2
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro WriteRegDWORD2

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Writes a registry string to HKCR using the supplied params and logs the
 * action to the install log and the uninstall log if _LOG_UNINSTALL equals 1.
 *
 * Define NO_LOG to prevent all logging when calling this from the uninstaller.
 *
 * @param   _ROOT
 *          The registry key root as defined by NSIS (e.g. HKLM, HKCU, etc.).
 *          This will only be used for logging.
 * @param   _KEY
 *          The subkey in relation to the key root.
 * @param   _NAME
 *          The key value name to write to.
 * @param   _STR
 *          The string to write to the key value name.
 * @param   _LOG_UNINSTALL
 *          0 = don't add to uninstall log, 1 = add to uninstall log.
 *
 * $R5 = _ROOT
 * $R6 = _KEY
 * $R7 = _NAME
 * $R8 = _STR
 * $R9 = _LOG_UNINSTALL
 */
!macro WriteRegStrHKCR

  !ifndef ${_MOZFUNC_UN}WriteRegStrHKCR
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}WriteRegStrHKCR "!insertmacro ${_MOZFUNC_UN}WriteRegStrHKCRCall"

    Function ${_MOZFUNC_UN}WriteRegStrHKCR
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Exch 3
      Exch $R6
      Exch 4
      Exch $R5

      ClearErrors
      WriteRegStr HKCR "$R6" "$R7" "$R8"

      !ifndef NO_LOG
        IfErrors 0 +3
        FileWrite $fhInstallLog "  ** ERROR Adding Registry String: $R5 | $R6 | $R7 | $R8 **$\r$\n"
        GoTo +4
        StrCmp "$R9" "1" +1 +2
        FileWrite $fhUninstallLog "RegVal: $R5 | $R6 | $R7$\r$\n"
        FileWrite $fhInstallLog "  Added Registry String: $R5 | $R6 | $R7 | $R8$\r$\n"
      !endif

      Exch $R5
      Exch 4
      Exch $R6
      Exch 3
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro WriteRegStrHKCRCall _ROOT _KEY _NAME _STR _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_NAME}"
  Push "${_STR}"
  Push "${_LOG_UNINSTALL}"
  Call WriteRegStrHKCR
  !verbose pop
!macroend

!macro un.WriteRegStrHKCRCall _ROOT _KEY _NAME _STR _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_NAME}"
  Push "${_STR}"
  Push "${_LOG_UNINSTALL}"
  Call un.WriteRegStrHKCR
  !verbose pop
!macroend

!macro un.WriteRegStrHKCR
  !ifndef un.WriteRegStrHKCR
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro WriteRegStrHKCR

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Creates a registry key. NSIS doesn't supply a RegCreateKey method and instead
 * will auto create keys when a reg key name value pair is set.
 * i - int (includes char, byte, short, handles, pointers and so on)
 * t - text, string (LPCSTR, pointer to first character)
 * * - pointer specifier -> the proc needs the pointer to type, affects next
 *     char (parameter) [ex: '*i' - pointer to int]
 * see the NSIS documentation for additional information.
 */
!define RegCreateKey "Advapi32::RegCreateKeyA(i, t, *i) i"

/**
 * Creates a registry key. This will log the actions to the install and
 * uninstall logs. Alternatively you can set a registry value to create the key
 * and then delete the value.
 *
 * Define NO_LOG to prevent all logging when calling this from the uninstaller.
 *
 * @param   _ROOT
 *          The registry key root as defined by NSIS (e.g. HKLM, HKCU, etc.).
 * @param   _KEY
 *          The subkey in relation to the key root.
 * @param   _LOG_UNINSTALL
 *          0 = don't add to uninstall log, 1 = add to uninstall log.
 *
 * $R4 = [out] handle to newly created registry key. If this is not a key
 *       located in one of the predefined registry keys this must be closed
 *       with RegCloseKey (this should not be needed unless someone decides to
 *       do something extremely squirrelly with NSIS).
 * $R5 = return value from RegCreateKeyA (represented by r15 in the system call).
 * $R6 = [in] hKey passed to RegCreateKeyA.
 * $R7 = _ROOT
 * $R8 = _KEY
 * $R9 = _LOG_UNINSTALL
 */
!macro CreateRegKey

  !ifndef ${_MOZFUNC_UN}CreateRegKey
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}CreateRegKey "!insertmacro ${_MOZFUNC_UN}CreateRegKeyCall"

    Function ${_MOZFUNC_UN}CreateRegKey
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Push $R6
      Push $R5
      Push $R4

      StrCmp $R7 "HKCR" +1 +2
      StrCpy $R6 "0x80000000"
      StrCmp $R7 "HKCU" +1 +2
      StrCpy $R6 "0x80000001"
      StrCmp $R7 "HKLM" +1 +2
      StrCpy $R6 "0x80000002"

      ; see definition of RegCreateKey
      System::Call "${RegCreateKey}($R6, '$R8', .r14) .r15"

      !ifndef NO_LOG
        ; if $R5 is not 0 then there was an error creating the registry key.
        IntCmp $R5 0 +3 +3
        FileWrite $fhInstallLog "  ** ERROR Adding Registry Key: $R7 | $R8 **$\r$\n"
        GoTo +4
        StrCmp "$R9" "1" +1 +2
        FileWrite $fhUninstallLog "RegKey: $R7 | $R8$\r$\n"
        FileWrite $fhInstallLog "  Added Registry Key: $R7 | $R8$\r$\n"
      !endif

      Pop $R4
      Pop $R5
      Pop $R6
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CreateRegKeyCall _ROOT _KEY _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_LOG_UNINSTALL}"
  Call CreateRegKey
  !verbose pop
!macroend

!macro un.CreateRegKeyCall _ROOT _KEY _LOG_UNINSTALL
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_ROOT}"
  Push "${_KEY}"
  Push "${_LOG_UNINSTALL}"
  Call un.CreateRegKey
  !verbose pop
!macroend

!macro un.CreateRegKey
  !ifndef un.CreateRegKey
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro CreateRegKey

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend


################################################################################
# Macros for adding file and protocol handlers

/**
 * Writes common registry values for a handler using SHCTX.
 *
 * @param   _KEY
 *          The subkey in relation to the key root.
 * @param   _VALOPEN
 *          The path and args to launch the application.
 * @param   _VALICON
 *          The path to an exe that contains an icon and the icon resource id.
 * @param   _DISPNAME
 *          The display name for the handler. If emtpy no value will be set.
 * @param   _ISPROTOCOL
 *          Sets protocol handler specific registry values when "true".
 * @param   _ISDDE
 *          Sets DDE specific registry values when "true".
 *
 * $R3 = string value of the current registry key path.
 * $R4 = _KEY
 * $R5 = _VALOPEN
 * $R6 = _VALICON
 * $R7 = _DISPNAME
 * $R8 = _ISPROTOCOL
 * $R9 = _ISDDE
 */
!macro AddHandlerValues

  !ifndef ${_MOZFUNC_UN}AddHandlerValues
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}AddHandlerValues "!insertmacro ${_MOZFUNC_UN}AddHandlerValuesCall"

    Function ${_MOZFUNC_UN}AddHandlerValues
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Exch 3
      Exch $R6
      Exch 4
      Exch $R5
      Exch 5
      Exch $R4
      Push $R3

      StrCmp "$R7" "" +6 +1
      ReadRegStr $R3 SHCTX "$R4" "FriendlyTypeName"

      StrCmp "$R3" "" +1 +3
      WriteRegStr SHCTX "$R4" "" "$R7"
      WriteRegStr SHCTX "$R4" "FriendlyTypeName" "$R7"

      StrCmp "$R8" "true" +1 +8
      WriteRegStr SHCTX "$R4" "URL Protocol" ""
      StrCpy $R3 ""
      ClearErrors
      ReadRegDWord $R3 SHCTX "$R4" "EditFlags"
      StrCmp $R3 "" +1 +3  ; Only add EditFlags if a value doesn't exist
      DeleteRegValue SHCTX "$R4" "EditFlags"
      WriteRegDWord SHCTX "$R4" "EditFlags" 0x00000002
      
      StrCmp "$R6" "" +2 +1
      WriteRegStr SHCTX "$R4\DefaultIcon" "" "$R6"
      
      StrCmp "$R5" "" +2 +1
      WriteRegStr SHCTX "$R4\shell\open\command" "" "$R5"      

!ifdef DDEApplication
      StrCmp "$R9" "true" +1 +11
      WriteRegStr SHCTX "$R4\shell\open\ddeexec" "" "$\"%1$\",,0,0,,,,"
      WriteRegStr SHCTX "$R4\shell\open\ddeexec" "NoActivateHandler" ""
      WriteRegStr SHCTX "$R4\shell\open\ddeexec\Application" "" "${DDEApplication}"
      WriteRegStr SHCTX "$R4\shell\open\ddeexec\Topic" "" "WWW_OpenURL"
      ; The ifexec key may have been added by another application so try to
      ; delete it to prevent it from breaking this app's shell integration.
      ; Also, IE 6 and below doesn't remove this key when it sets itself as the
      ; default handler and if this key exists IE's shell integration breaks.
      DeleteRegKey HKLM "$R4\shell\open\ddeexec\ifexec"
      DeleteRegKey HKCU "$R4\shell\open\ddeexec\ifexec"
!endif

      ClearErrors

      Pop $R3
      Exch $R4
      Exch 5
      Exch $R5
      Exch 4
      Exch $R6
      Exch 3
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro AddHandlerValuesCall _KEY _VALOPEN _VALICON _DISPNAME _ISPROTOCOL _ISDDE
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Push "${_VALOPEN}"
  Push "${_VALICON}"
  Push "${_DISPNAME}"
  Push "${_ISPROTOCOL}"
  Push "${_ISDDE}"
  Call AddHandlerValues
  !verbose pop
!macroend

!macro un.AddHandlerValuesCall _KEY _VALOPEN _VALICON _DISPNAME _ISPROTOCOL _ISDDE
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Push "${_VALOPEN}"
  Push "${_VALICON}"
  Push "${_DISPNAME}"
  Push "${_ISPROTOCOL}"
  Push "${_ISDDE}"
  Call un.AddHandlerValues
  !verbose pop
!macroend

!macro un.AddHandlerValues
  !ifndef un.AddHandlerValues
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro AddHandlerValues

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Writes common registry values for a handler that uses DDE using SHCTX.
 *
 * @param   _KEY
 *          The key name in relation to the HKCR root. SOFTWARE\Classes is
 *          prefixed to this value when using SHCTX.
 * @param   _VALOPEN
 *          The path and args to launch the application.
 * @param   _VALICON
 *          The path to an exe that contains an icon and the icon resource id.
 * @param   _DISPNAME
 *          The display name for the handler. If emtpy no value will be set.
 * @param   _ISPROTOCOL
 *          Sets protocol handler specific registry values when "true".
 * @param   _DDE_APPNAME
 *          Sets DDE specific registry values when not an empty string.
 *
 * $R0 = storage for SOFTWARE\Classes
 * $R1 = string value of the current registry key path.
 * $R2 = _KEY
 * $R3 = _VALOPEN
 * $R4 = _VALICON
 * $R5 = _DISPNAME
 * $R6 = _ISPROTOCOL
 * $R7 = _DDE_APPNAME
 * $R8 = _DDE_DEFAULT
 * $R9 = _DDE_TOPIC
 */
!macro AddDDEHandlerValues

  !ifndef ${_MOZFUNC_UN}AddDDEHandlerValues
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}AddDDEHandlerValues "!insertmacro ${_MOZFUNC_UN}AddDDEHandlerValuesCall"

    Function ${_MOZFUNC_UN}AddDDEHandlerValues
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Exch 3
      Exch $R6
      Exch 4
      Exch $R5
      Exch 5
      Exch $R4
      Exch 6
      Exch $R3
      Exch 7
      Exch $R2
      Push $R1
      Push $R0

      StrCpy $R0 "SOFTWARE\Classes"
      StrCmp "$R5" "" +6 +1
      ReadRegStr $R1 SHCTX "$R2" "FriendlyTypeName"

      StrCmp "$R1" "" +1 +3
      WriteRegStr SHCTX "$R0\$R2" "" "$R5"
      WriteRegStr SHCTX "$R0\$R2" "FriendlyTypeName" "$R5"

      StrCmp "$R6" "true" +1 +8
      WriteRegStr SHCTX "$R0\$R2" "URL Protocol" ""
      StrCpy $R1 ""
      ClearErrors
      ReadRegDWord $R1 SHCTX "$R0\$R2" "EditFlags"
      StrCmp $R1 "" +1 +3  ; Only add EditFlags if a value doesn't exist
      DeleteRegValue SHCTX "$R0\$R2" "EditFlags"
      WriteRegDWord SHCTX "$R0\$R2" "EditFlags" 0x00000002
      
      StrCmp "$R4" "" +2 +1
      WriteRegStr SHCTX "$R0\$R2\DefaultIcon" "" "$R4"

      WriteRegStr SHCTX "$R0\$R2\shell\open\command" "" "$R3"

      WriteRegStr SHCTX "$R0\$R2\shell\open\ddeexec" "" "$R8"
      WriteRegStr SHCTX "$R0\$R2\shell\open\ddeexec" "NoActivateHandler" ""
      WriteRegStr SHCTX "$R0\$R2\shell\open\ddeexec\Application" "" "$R7"
      WriteRegStr SHCTX "$R0\$R2\shell\open\ddeexec\Topic" "" "$R9"

      ; The ifexec key may have been added by another application so try to
      ; delete it to prevent it from breaking this app's shell integration.
      ; Also, IE 6 and below doesn't remove this key when it sets itself as the
      ; default handler and if this key exists IE's shell integration breaks.
      DeleteRegKey HKLM "$R0\$R2\shell\open\ddeexec\ifexec"
      DeleteRegKey HKCU "$R0\$R2\shell\open\ddeexec\ifexec"
      ClearErrors

      Pop $R0
      Pop $R1
      Exch $R2
      Exch 7
      Exch $R3
      Exch 6
      Exch $R4
      Exch 5
      Exch $R5
      Exch 4
      Exch $R6
      Exch 3
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro AddDDEHandlerValuesCall _KEY _VALOPEN _VALICON _DISPNAME _ISPROTOCOL _DDE_APPNAME _DDE_DEFAULT _DDE_TOPIC
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Push "${_VALOPEN}"
  Push "${_VALICON}"
  Push "${_DISPNAME}"
  Push "${_ISPROTOCOL}"
  Push "${_DDE_APPNAME}"
  Push "${_DDE_DEFAULT}"
  Push "${_DDE_TOPIC}"
  Call AddDDEHandlerValues
  !verbose pop
!macroend

!macro un.AddDDEHandlerValuesCall _KEY _VALOPEN _VALICON _DISPNAME _ISPROTOCOL _DDE_APPNAME _DDE_DEFAULT _DDE_TOPIC
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Push "${_VALOPEN}"
  Push "${_VALICON}"
  Push "${_DISPNAME}"
  Push "${_ISPROTOCOL}"
  Push "${_DDE_APPNAME}"
  Push "${_DDE_DEFAULT}"
  Push "${_DDE_TOPIC}"
  Call un.AddDDEHandlerValues
  !verbose pop
!macroend

!macro un.AddDDEHandlerValues
  !ifndef un.AddDDEHandlerValues
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro AddDDEHandlerValues

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

################################################################################
# Macros for retrieving existing install paths

/**
 * Finds a second installation of the application so we can make informed
 * decisions about registry operations. This uses SHCTX to determine the
 * registry hive so you must call SetShellVarContext first.
 *
 * @param   _KEY
 *          The registry subkey (typically this will be Software\Mozilla).
 * @return  _RESULT
 *          false if a second install isn't found, path to the main exe if a
 *          second install is found.
 *
 * $R3 = stores the long path to $INSTDIR
 * $R4 = counter for the outer loop's EnumRegKey
 * $R5 = return value from ReadRegStr and RemoveQuotesFromPath
 * $R6 = return value from GetParent
 * $R7 = return value from the loop's EnumRegKey
 * $R8 = storage for _KEY
 * $R9 = _KEY and _RESULT
 */
!macro GetSecondInstallPath

  !ifndef ${_MOZFUNC_UN}GetSecondInstallPath
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetLongPath
    !insertmacro ${_MOZFUNC_UN_TMP}GetParent
    !insertmacro ${_MOZFUNC_UN_TMP}RemoveQuotesFromPath
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}GetSecondInstallPath "!insertmacro ${_MOZFUNC_UN}GetSecondInstallPathCall"

    Function ${_MOZFUNC_UN}GetSecondInstallPath
      Exch $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4
      Push $R3

      ${${_MOZFUNC_UN}GetLongPath} "$INSTDIR" $R3

      StrCpy $R4 0       ; set the counter for the loop to 0
      StrCpy $R8 "$R9"   ; Registry key path to search
      StrCpy $R9 "false" ; default return value

      loop:
      EnumRegKey $R7 SHCTX $R8 $R4
      StrCmp $R7 "" end +1  ; if empty there are no more keys to enumerate
      IntOp $R4 $R4 + 1     ; increment the loop's counter
      ClearErrors
      ReadRegStr $R5 SHCTX "$R8\$R7\bin" "PathToExe"
      IfErrors loop

      ${${_MOZFUNC_UN}RemoveQuotesFromPath} "$R5" $R5

      IfFileExists "$R5" +1 loop
      ${${_MOZFUNC_UN}GetLongPath} "$R5" $R5
      ${${_MOZFUNC_UN}GetParent} "$R5" $R6
      StrCmp "$R6" "$R3" loop +1
      StrCmp "$R6\${FileMainEXE}" "$R5" +1 loop
      StrCpy $R9 "$R5"

      end:
      ClearErrors

      Pop $R3
      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro GetSecondInstallPathCall _KEY _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Call GetSecondInstallPath
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.GetSecondInstallPathCall _KEY _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Call un.GetSecondInstallPath
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.GetSecondInstallPath
  !ifndef un.GetSecondInstallPath
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro GetSecondInstallPath

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Finds an existing installation path for the application based on the
 * application's executable name so we can default to using this path for the
 * install. If there is zero or more than one installation of the application
 * then we default to the default installation path. This uses SHCTX to
 * determine the registry hive to read from so you must call SetShellVarContext
 * first.
 *
 * @param   _KEY
 *          The registry subkey (typically this will be Software\Mozilla\App Name).
 * @return  _RESULT
 *          false if a single install location for this app name isn't found,
 *          path to the install directory if a single install location is found.
 *
 * $R5 = counter for the loop's EnumRegKey
 * $R6 = return value from EnumRegKey
 * $R7 = return value from ReadRegStr
 * $R8 = storage for _KEY
 * $R9 = _KEY and _RESULT
 */
!macro GetSingleInstallPath

  !ifndef ${_MOZFUNC_UN}GetSingleInstallPath
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetLongPath
    !insertmacro ${_MOZFUNC_UN_TMP}GetParent
    !insertmacro ${_MOZFUNC_UN_TMP}RemoveQuotesFromPath
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}GetSingleInstallPath "!insertmacro ${_MOZFUNC_UN}GetSingleInstallPathCall"

    Function ${_MOZFUNC_UN}GetSingleInstallPath
      Exch $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5

      StrCpy $R8 $R9
      StrCpy $R9 "false"
      StrCpy $R5 0  ; set the counter for the loop to 0

      loop:
      ClearErrors
      EnumRegKey $R6 SHCTX $R8 $R5
      IfErrors cleanup
      StrCmp $R6 "" cleanup +1  ; if empty there are no more keys to enumerate
      IntOp $R5 $R5 + 1         ; increment the loop's counter
      ClearErrors
      ReadRegStr $R7 SHCTX "$R8\$R6\Main" "PathToExe"
      IfErrors loop
      ${${_MOZFUNC_UN}RemoveQuotesFromPath} "$R7" $R7
      GetFullPathName $R7 "$R7"
      IfErrors loop

      StrCmp "$R9" "false" +1 +3
      StrCpy $R9 "$R7"
      GoTo Loop

      StrCpy $R9 "false"

      cleanup:
      StrCmp $R9 "false" end +1
      ${${_MOZFUNC_UN}GetLongPath} "$R9" $R9
      ${${_MOZFUNC_UN}GetParent} "$R9" $R9

      end:
      ClearErrors

      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro GetSingleInstallPathCall _KEY _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Call GetSingleInstallPath
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.GetSingleInstallPathCall _KEY _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Call un.GetSingleInstallPath
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.GetSingleInstallPath
  !ifndef un.GetSingleInstallPath
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro GetSingleInstallPath

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend


################################################################################
# Macros for working with the file system

/**
 * Attempts to delete a file if it exists. This will fail if the file is in use.
 *
 * @param   _FILE
 *          The path to the file that is to be deleted.
 */
!macro DeleteFile _FILE
  ${If} ${FileExists} "${_FILE}"
    Delete "${_FILE}"
  ${EndIf}
!macroend
!define DeleteFile "!insertmacro DeleteFile"

/**
 * Removes a directory if it exists and is empty.
 *
 * @param   _DIR
 *          The path to the directory that is to be removed.
 */
!macro RemoveDir _DIR
  ${If} ${FileExists} "${_DIR}"
    RmDir "${_DIR}"
  ${EndIf}
!macroend
!define RemoveDir "!insertmacro RemoveDir"

/**
 * Checks whether we can write to the install directory. If the install
 * directory already exists this will attempt to create a temporary file in the
 * install directory and then delete it. If it does not exist this will attempt
 * to create the directory and then delete it. If we can write to the install
 * directory this will return true... if not, this will return false.
 *
 * @return  _RESULT
 *          true if the install directory can be written to otherwise false.
 *
 * $R7 = temp filename in installation directory returned from GetTempFileName
 * $R8 = filehandle to temp file used for writing
 * $R9 = _RESULT
 */
!macro CanWriteToInstallDir

  !ifndef ${_MOZFUNC_UN}CanWriteToInstallDir
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}CanWriteToInstallDir "!insertmacro ${_MOZFUNC_UN}CanWriteToInstallDirCall"

    Function ${_MOZFUNC_UN}CanWriteToInstallDir
      Push $R9
      Push $R8
      Push $R7

      StrCpy $R9 "true"
      IfFileExists "$INSTDIR" +1 checkCreateDir
      GetTempFileName $R7 "$INSTDIR"
      FileOpen $R8 $R7 w
      FileWrite $R8 "Write Access Test"
      FileClose $R8
      IfFileExists "$R7" +3 +1
      StrCpy $R9 "false"
      GoTo end

      Delete $R7
      GoTo end

      checkCreateDir:
      CreateDirectory "$INSTDIR"
      IfFileExists "$INSTDIR" +3 +1
      StrCpy $R9 "false"
      GoTo end

      RmDir "$INSTDIR"

      end:
      ClearErrors

      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CanWriteToInstallDirCall _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call CanWriteToInstallDir
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.CanWriteToInstallDirCall _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.CanWriteToInstallDir
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.CanWriteToInstallDir
  !ifndef un.CanWriteToInstallDir
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro CanWriteToInstallDir

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Checks whether there is sufficient free space available on the installation
 * directory's drive. If there is sufficient free space this will return true...
 * if not, this will return false. This will only calculate the size of the
 * first three sections.
 *
 * @return  _RESULT
 *          true if there is sufficient free space otherwise false.
 *
 * $R2 = return value from greater than comparison (0=false 1=true)
 * $R3 = free space for the install directory's drive
 * $R4 = install directory root
 * $R5 = return value from SectionGetSize
 * $R6 = return value from 'and' comparison of SectionGetFlags (1=selected)
 * $R7 = return value from SectionGetFlags
 * $R8 = size in KB required for this installation
 * $R9 = _RESULT
 */
!macro CheckDiskSpace

  !ifndef ${_MOZFUNC_UN}CheckDiskSpace
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetRoot
    !insertmacro ${_MOZFUNC_UN_TMP}DriveSpace
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}CheckDiskSpace "!insertmacro ${_MOZFUNC_UN}CheckDiskSpaceCall"

    Function ${_MOZFUNC_UN}CheckDiskSpace
      Push $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2

      StrCpy $R9 "true"
      SectionGetSize 0 $R8

      SectionGetFlags 1 $R7
      IntOp $R6 ${SF_SELECTED} & $R7
      IntCmp $R6 0 +3 +1 +1
      SectionGetSize 1 $R5
      IntOp $R8 $R8 + $R5

      SectionGetFlags 2 $R7
      IntOp $R6 ${SF_SELECTED} & $R7
      IntCmp $R6 0 +3 +1 +1
      SectionGetSize 2 $R5
      IntOp $R8 $R8 + $R5

      ${${_MOZFUNC_UN}GetRoot} "$INSTDIR" $R4
      ${${_MOZFUNC_UN}DriveSpace} "$R4" "/D=F /S=K" $R3

      System::Int64Op $R3 > $R8
      Pop $R2

      IntCmp $R2 1 end +1 +1
      StrCpy $R9 "false"

      end:
      ClearErrors

      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CheckDiskSpaceCall _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call CheckDiskSpace
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.CheckDiskSpaceCall _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.CheckDiskSpace
  Pop ${_RESULT}
  !verbose pop
!macroend

!macro un.CheckDiskSpace
  !ifndef un.CheckDiskSpace
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro CheckDiskSpace

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
* Returns the path found within a passed in string. The path is quoted or not
* with the exception of an unquoted non 8dot3 path without arguments that is
* also not a DefaultIcon path, is a 8dot3 path or not, has command line
* arguments, or is a registry DefaultIcon path (e.g. <path to binary>,# where #
* is the icon's resuorce id). The string does not need to be a valid path or
* exist. It is up to the caller to pass in a string of one of the forms noted
* above and to verify existence if necessary.
*
* Examples:
* In:  C:\PROGRA~1\MOZILL~1\FIREFOX.EXE -flag "%1"
* In:  C:\PROGRA~1\MOZILL~1\FIREFOX.EXE,0
* In:  C:\PROGRA~1\MOZILL~1\FIREFOX.EXE
* In:  "C:\PROGRA~1\MOZILL~1\FIREFOX.EXE"
* In:  "C:\PROGRA~1\MOZILL~1\FIREFOX.EXE" -flag "%1"
* Out: C:\PROGRA~1\MOZILL~1\FIREFOX.EXE
*
* In:  "C:\Program Files\Mozilla Firefox\firefox.exe" -flag "%1"
* In:  C:\Program Files\Mozilla Firefox\firefox.exe,0
* In:  "C:\Program Files\Mozilla Firefox\firefox.exe"
* Out: C:\Program Files\Mozilla Firefox\firefox.exe
*
* @param   _IN_PATH
*          The string containing the path.
* @param   _OUT_PATH
*          The register to store the path to.
*
* $R7 = counter for the outer loop's EnumRegKey
* $R8 = return value from ReadRegStr
* $R9 = _IN_PATH and _OUT_PATH
*/
!macro GetPathFromString

  !ifndef ${_MOZFUNC_UN}GetPathFromString
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}GetPathFromString "!insertmacro ${_MOZFUNC_UN}GetPathFromStringCall"

    Function ${_MOZFUNC_UN}GetPathFromString
      Exch $R9
      Push $R8
      Push $R7

      StrCpy $R7 0          ; Set the counter to 0.

      ; Handle quoted paths with arguments.
      StrCpy $R8 $R9 1      ; Copy the first char.
      StrCmp $R8 '"' +2 +1  ; Is it a "?
      StrCmp $R8 "'" +1 +9  ; Is it a '?
      StrCpy $R9 $R9 "" 1   ; Remove the first char.
      IntOp $R7 $R7 + 1     ; Increment the counter.
      StrCpy $R8 $R9 1 $R7  ; Starting from the counter copy the next char.
      StrCmp $R8 "" end +1  ; Are there no more chars?
      StrCmp $R8 '"' +2 +1  ; Is it a " char?
      StrCmp $R8 "'" +1 -4  ; Is it a ' char?
      StrCpy $R9 $R9 $R7    ; Copy chars up to the counter.
      GoTo end

      ; Handle DefaultIcon paths. DefaultIcon paths are not quoted and end with
      ; a , and a number.
      IntOp $R7 $R7 - 1     ; Decrement the counter.
      StrCpy $R8 $R9 1 $R7  ; Copy one char from the end minus the counter.
      StrCmp $R8 '' +4 +1   ; Are there no more chars?
      StrCmp $R8 ',' +1 -3  ; Is it a , char?
      StrCpy $R9 $R9 $R7    ; Copy chars up to the end minus the counter.
      GoTo end

      ; Handle unquoted paths with arguments. An unquoted path with arguments
      ; must be an 8dot3 path.
      StrCpy $R7 -1          ; Set the counter to -1 so it will start at 0.
      IntOp $R7 $R7 + 1      ; Increment the counter.
      StrCpy $R8 $R9 1 $R7   ; Starting from the counter copy the next char.
      StrCmp $R8 "" end +1   ; Are there no more chars?
      StrCmp $R8 " " +1 -3   ; Is it a space char?
      StrCpy $R9 $R9 $R7     ; Copy chars up to the counter.

      end:
      ClearErrors

      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro GetPathFromStringCall _IN_PATH _OUT_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_IN_PATH}"
  Call GetPathFromString
  Pop ${_OUT_PATH}
  !verbose pop
!macroend

!macro un.GetPathFromStringCall _IN_PATH _OUT_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_IN_PATH}"
  Call un.GetPathFromString
  Pop ${_OUT_PATH}
  !verbose pop
!macroend

!macro un.GetPathFromString
  !ifndef un.GetPathFromString
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro GetPathFromString

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Removes the quotes from each end of a string if present.
 *
 * @param   _IN_PATH
 *          The string containing the path.
 * @param   _OUT_PATH
 *          The register to store the long path.
 *
 * $R7 = storage for single character comparison
 * $R8 = storage for _IN_PATH
 * $R9 = _IN_PATH and _OUT_PATH
 */
!macro RemoveQuotesFromPath

  !ifndef ${_MOZFUNC_UN}RemoveQuotesFromPath
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}RemoveQuotesFromPath "!insertmacro ${_MOZFUNC_UN}RemoveQuotesFromPathCall"

    Function ${_MOZFUNC_UN}RemoveQuotesFromPath
      Exch $R9
      Push $R8
      Push $R7

      StrCpy $R7 "$R9" 1
      StrCmp $R7 "$\"" +1 +2
      StrCpy $R9 "$R9" "" 1

      StrCpy $R7 "$R9" "" -1
      StrCmp $R7 "$\"" +1 +2
      StrCpy $R9 "$R9" -1

      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro RemoveQuotesFromPathCall _IN_PATH _OUT_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_IN_PATH}"
  Call RemoveQuotesFromPath
  Pop ${_OUT_PATH}
  !verbose pop
!macroend

!macro un.RemoveQuotesFromPathCall _IN_PATH _OUT_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_IN_PATH}"
  Call un.RemoveQuotesFromPath
  Pop ${_OUT_PATH}
  !verbose pop
!macroend

!macro un.RemoveQuotesFromPath
  !ifndef un.RemoveQuotesFromPath
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro RemoveQuotesFromPath

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Returns the long path for an existing file or directory. GetLongPathNameA
 * may not be available on Win95 if Microsoft Layer for Unicode is not
 * installed and GetFullPathName only returns a long path for the last file or
 * directory that doesn't end with a \ in the path that it is passed. If the
 * path does not exist on the file system this will return an empty string. To
 * provide a consistent result trailing back-slashes are always removed.
 *
 * Note: 1024 used by GetLongPathNameA is the maximum NSIS string length.
 *
 * @param   _IN_PATH
 *          The string containing the path.
 * @param   _OUT_PATH
 *          The register to store the long path.
 *
 * $R4 = counter value when the previous \ was found
 * $R5 = directory or file name found during loop
 * $R6 = return value from GetLongPathNameA and loop counter
 * $R7 = long path from GetLongPathNameA and single char from path for comparison
 * $R8 = storage for _IN_PATH
 * $R9 = _IN_PATH _OUT_PATH
 */
!macro GetLongPath

  !ifndef ${_MOZFUNC_UN}GetLongPath
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}GetLongPath "!insertmacro ${_MOZFUNC_UN}GetLongPathCall"

    Function ${_MOZFUNC_UN}GetLongPath
      Exch $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4

      ClearErrors

      StrCpy $R8 "$R9"
      StrCpy $R9 ""
      GetFullPathName $R8 "$R8"
      IfErrors end_GetLongPath +1 ; If the path doesn't exist return an empty string.

      ; Remove trailing \'s from the path.
      StrCpy $R6 "$R8" "" -1
      StrCmp $R6 "\" +1 +2
      StrCpy $R9 "$R8" -1

      System::Call 'kernel32::GetLongPathNameA(t r18, t .r17, i 1024)i .r16'
      StrCmp "$R7" "" +4 +1 ; Empty string when GetLongPathNameA is not present.
      StrCmp $R6 0 +3 +1    ; Should never equal 0 since the path exists.
      StrCpy $R9 "$R7"
      GoTo end_GetLongPath

      ; Do it the hard way.
      StrCpy $R4 0     ; Stores the position in the string of the last \ found.
      StrCpy $R6 -1    ; Set the counter to -1 so it will start at 0.

      loop_GetLongPath:
      IntOp $R6 $R6 + 1      ; Increment the counter.
      StrCpy $R7 $R8 1 $R6   ; Starting from the counter copy the next char.
      StrCmp $R7 "" +2 +1    ; Are there no more chars?
      StrCmp $R7 "\" +1 -3   ; Is it a \?

      ; Copy chars starting from the previously found \ to the counter.
      StrCpy $R5 $R8 $R6 $R4

      ; If this is the first \ found we want to swap R9 with R5 so a \ will
      ; be appended to the drive letter and colon (e.g. C: will become C:\).
      StrCmp $R4 0 +1 +3     
      StrCpy $R9 $R5
      StrCpy $R5 ""

      GetFullPathName $R9 "$R9\$R5"

      StrCmp $R7 "" end_GetLongPath +1 ; Are there no more chars?

      ; Store the counter for the current \ and prefix it for StrCpy operations.
      StrCpy $R4 "+$R6"
      IntOp $R6 $R6 + 1      ; Increment the counter so we skip over the \.
      StrCpy $R8 $R8 "" $R6  ; Copy chars starting from the counter to the end.
      StrCpy $R6 -1          ; Reset the counter to -1 so it will start over at 0.
      GoTo loop_GetLongPath

      end_GetLongPath:
      ClearErrors

      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro GetLongPathCall _IN_PATH _OUT_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_IN_PATH}"
  Call GetLongPath
  Pop ${_OUT_PATH}
  !verbose pop
!macroend

!macro un.GetLongPathCall _IN_PATH _OUT_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_IN_PATH}"
  Call un.GetLongPath
  Pop ${_OUT_PATH}
  !verbose pop
!macroend

!macro un.GetLongPath
  !ifndef un.GetLongPath
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro GetLongPath

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend


################################################################################
# Macros for cleaning up the registry and file system

/**
 * Removes registry keys that reference this install location and for paths that
 * no longer exist. This uses SHCTX to determine the registry hive so you must
 * call SetShellVarContext first.
 *
 * @param   _KEY
 *          The registry subkey (typically this will be Software\Mozilla).
 *
 * XXXrstrong - there is the potential for Key: Software/Mozilla/AppName,
 * ValueName: CurrentVersion, ValueData: AppVersion to reference a key that is
 * no longer available due to this cleanup. This should be no worse than prior
 * to this reg cleanup since the referenced key would be for an app that is no
 * longer installed on the system.
 *
 * $R1 = stores the long path to $INSTDIR
 * $R2 = return value from the stack from the GetParent and GetLongPath macros
 * $R3 = return value from the outer loop's EnumRegKey
 * $R4 = return value from the inner loop's EnumRegKey
 * $R5 = return value from ReadRegStr
 * $R6 = counter for the outer loop's EnumRegKey
 * $R7 = counter for the inner loop's EnumRegKey
 * $R8 = return value from the stack from the RemoveQuotesFromPath macro
 * $R9 = _KEY
 */
!macro RegCleanMain

  !ifndef ${_MOZFUNC_UN}RegCleanMain
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetParent
    !insertmacro ${_MOZFUNC_UN_TMP}GetLongPath
    !insertmacro ${_MOZFUNC_UN_TMP}RemoveQuotesFromPath
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}RegCleanMain "!insertmacro ${_MOZFUNC_UN}RegCleanMainCall"

    Function ${_MOZFUNC_UN}RegCleanMain
      Exch $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2
      Push $R1

      ${${_MOZFUNC_UN}GetLongPath} "$INSTDIR" $R1
      StrCpy $R6 0  ; set the counter for the outer loop to 0

      outerloop:
      EnumRegKey $R3 SHCTX $R9 $R6
      StrCmp $R3 "" end +1  ; if empty there are no more keys to enumerate
      IntOp $R6 $R6 + 1     ; increment the outer loop's counter
      ClearErrors
      ReadRegStr $R5 SHCTX "$R9\$R3\bin" "PathToExe"
      IfErrors 0 outercontinue
      StrCpy $R7 0  ; set the counter for the inner loop to 0

      innerloop:
      EnumRegKey $R4 SHCTX "$R9\$R3" $R7
      StrCmp $R4 "" outerloop +1  ; if empty there are no more keys to enumerate
      IntOp $R7 $R7 + 1  ; increment the inner loop's counter
      ClearErrors
      ReadRegStr $R5 SHCTX "$R9\$R3\$R4\Main" "PathToExe"
      IfErrors innerloop

      ${${_MOZFUNC_UN}RemoveQuotesFromPath} "$R5" $R8
      ${${_MOZFUNC_UN}GetParent} "$R8" $R2
      ${${_MOZFUNC_UN}GetLongPath} "$R2" $R2
      IfFileExists "$R2" +1 innerloop
      StrCmp "$R2" "$R1" +1 innerloop

      ClearErrors
      DeleteRegKey SHCTX "$R9\$R3\$R4"
      IfErrors innerloop
      IntOp $R7 $R7 - 1 ; decrement the inner loop's counter when the key is deleted successfully.
      ClearErrors
      DeleteRegKey /ifempty SHCTX "$R9\$R3"
      IfErrors innerloop outerdecrement

      outercontinue:
      ${${_MOZFUNC_UN}RemoveQuotesFromPath} "$R5" $R8
      ${${_MOZFUNC_UN}GetParent} "$R8" $R2
      ${${_MOZFUNC_UN}GetLongPath} "$R2" $R2
      IfFileExists "$R2" +1 outerloop
      StrCmp "$R2" "$R1" +1 outerloop

      ClearErrors
      DeleteRegKey SHCTX "$R9\$R3"
      IfErrors outerloop

      outerdecrement:
      IntOp $R6 $R6 - 1 ; decrement the outer loop's counter when the key is deleted successfully.
      GoTo outerloop

      end:
      ClearErrors

      Pop $R1
      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro RegCleanMainCall _KEY
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Call RegCleanMain
  !verbose pop
!macroend

!macro un.RegCleanMainCall _KEY
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_KEY}"
  Call un.RegCleanMain
  !verbose pop
!macroend

!macro un.RegCleanMain
  !ifndef un.RegCleanMain
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro RegCleanMain

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Removes all registry keys from \Software\Windows\CurrentVersion\Uninstall
 * that reference this install location. This uses SHCTX to determine the
 * registry hive so you must call SetShellVarContext first.
 *
 * $R4 = stores the long path to $INSTDIR
 * $R5 = return value from ReadRegStr
 * $R6 = string for the base reg key (e.g. Software\Microsoft\Windows\CurrentVersion\Uninstall)
 * $R7 = return value from EnumRegKey
 * $R8 = counter for EnumRegKey
 * $R9 = return value from the stack from the RemoveQuotesFromPath and GetLongPath macros
 */
!macro RegCleanUninstall

  !ifndef ${_MOZFUNC_UN}RegCleanUninstall
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetLongPath
    !insertmacro ${_MOZFUNC_UN_TMP}RemoveQuotesFromPath
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}RegCleanUninstall "!insertmacro ${_MOZFUNC_UN}RegCleanUninstallCall"

    Function ${_MOZFUNC_UN}RegCleanUninstall
      Push $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4

      ${${_MOZFUNC_UN}GetLongPath} "$INSTDIR" $R4
      StrCpy $R6 "Software\Microsoft\Windows\CurrentVersion\Uninstall"
      StrCpy $R7 ""
      StrCpy $R8 0

      loop:
      EnumRegKey $R7 SHCTX $R6 $R8
      StrCmp $R7 "" end +1
      IntOp $R8 $R8 + 1 ; Increment the counter
      ClearErrors
      ReadRegStr $R5 SHCTX "$R6\$R7" "InstallLocation"
      IfErrors loop
      ${${_MOZFUNC_UN}RemoveQuotesFromPath} "$R5" $R9
      ${${_MOZFUNC_UN}GetLongPath} "$R9" $R9
      StrCmp "$R9" "$R4" +1 loop
      ClearErrors
      DeleteRegKey SHCTX "$R6\$R7"
      IfErrors loop
      IntOp $R8 $R8 - 1 ; Decrement the counter on successful deletion
      GoTo loop

      end:
      ClearErrors

      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Pop $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro RegCleanUninstallCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call RegCleanUninstall
  !verbose pop
!macroend

!macro un.RegCleanUninstallCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.RegCleanUninstall
  !verbose pop
!macroend

!macro un.RegCleanUninstall
  !ifndef un.RegCleanUninstall
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro RegCleanUninstall

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Removes an application specific handler registry key under Software\Classes
 * for both HKCU and HKLM when its open command refers to this install
 * location or the install location doesn't exist.
 *
 * @param   _HANDLER_NAME
 *          The registry name for the handler.
 *
 * $R7 = stores the long path to the $INSTDIR
 * $R8 = stores the path to the open command's parent directory
 * $R9 = _HANDLER_NAME
 */
!macro RegCleanAppHandler

  !ifndef ${_MOZFUNC_UN}RegCleanAppHandler
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetLongPath
    !insertmacro ${_MOZFUNC_UN_TMP}GetParent
    !insertmacro ${_MOZFUNC_UN_TMP}GetPathFromString
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}RegCleanAppHandler "!insertmacro ${_MOZFUNC_UN}RegCleanAppHandlerCall"

    Function ${_MOZFUNC_UN}RegCleanAppHandler
      Exch $R9
      Push $R8
      Push $R7

      ClearErrors
      ReadRegStr $R8 HKCU "Software\Classes\$R9\shell\open\command" ""
      IfErrors next +1
      ${${_MOZFUNC_UN}GetPathFromString} "$R8" $R8
      ${${_MOZFUNC_UN}GetParent} "$R8" $R8
      IfFileExists "$R8" +3 +1
      DeleteRegKey HKCU "Software\Classes\$R9"
      GoTo next

      ${${_MOZFUNC_UN}GetLongPath} "$R8" $R8
      ${${_MOZFUNC_UN}GetLongPath} "$INSTDIR" $R7
      StrCmp "$R7" "$R8" +1 next
      DeleteRegKey HKCU "Software\Classes\$R9"

      next:
      ReadRegStr $R8 HKLM "Software\Classes\$R9\shell\open\command" ""
      IfErrors end
      ${${_MOZFUNC_UN}GetPathFromString} "$R8" $R8
      ${${_MOZFUNC_UN}GetParent} "$R8" $R8
      IfFileExists "$R8" +3 +1
      DeleteRegKey HKLM "Software\Classes\$R9"
      GoTo end

      ${${_MOZFUNC_UN}GetLongPath} "$R8" $R8
      ${${_MOZFUNC_UN}GetLongPath} "$INSTDIR" $R7
      StrCmp "$R7" "$R8" +1 end
      DeleteRegKey HKLM "Software\Classes\$R9"

      end:

      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro RegCleanAppHandlerCall _HANDLER_NAME
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_HANDLER_NAME}"
  Call RegCleanAppHandler
  !verbose pop
!macroend

!macro un.RegCleanAppHandlerCall _HANDLER_NAME
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_HANDLER_NAME}"
  Call un.RegCleanAppHandler
  !verbose pop
!macroend

!macro un.RegCleanAppHandler
  !ifndef un.RegCleanAppHandler
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro RegCleanAppHandler

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Cleans up the registry for a protocol handler when its open command
 * refers to this install location. For HKCU the registry key is deleted
 * and for HKLM the values set by the application are deleted.
 *
 * @param   _HANDLER_NAME
 *          The registry name for the handler.
 *
 * $R7 = stores the long path to $INSTDIR
 * $R8 = stores the the long path to the open command's parent directory
 * $R9 = _HANDLER_NAME
 */
!macro un.RegCleanProtocolHandler

  !ifndef un.RegCleanProtocolHandler
    !insertmacro un.GetLongPath
    !insertmacro un.GetParent
    !insertmacro un.GetPathFromString

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define un.RegCleanProtocolHandler "!insertmacro un.RegCleanProtocolHandlerCall"

    Function un.RegCleanProtocolHandler
      Exch $R9
      Push $R8
      Push $R7

      ReadRegStr $R8 HKCU "Software\Classes\$R9\shell\open\command" ""
      ${un.GetLongPath} "$INSTDIR" $R7

      StrCmp "$R8" "" next +1
      ${un.GetPathFromString} "$R8" $R8
      ${un.GetParent} "$R8" $R8
      ${un.GetLongPath} "$R8" $R8
      StrCmp "$R7" "$R8" +1 next
      DeleteRegKey HKCU "Software\Classes\$R9"

      next:
      ReadRegStr $R8 HKLM "Software\Classes\$R9\shell\open\command" ""
      StrCmp "$R8" "" end +1
      ${un.GetLongPath} "$INSTDIR" $R7
      ${un.GetPathFromString} "$R8" $R8
      ${un.GetParent} "$R8" $R8
      ${un.GetLongPath} "$R8" $R8
      StrCmp "$R7" "$R8" +1 end
      DeleteRegValue HKLM "Software\Classes\$R9\DefaultIcon" ""
      DeleteRegValue HKLM "Software\Classes\$R9\shell\open" ""
      DeleteRegValue HKLM "Software\Classes\$R9\shell\ddeexec" ""
      DeleteRegValue HKLM "Software\Classes\$R9\shell\ddeexec\Application" ""
      DeleteRegValue HKLM "Software\Classes\$R9\shell\ddeexec\Topic" ""

      end:

      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro un.RegCleanProtocolHandlerCall _HANDLER_NAME
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_HANDLER_NAME}"
  Call un.RegCleanProtocolHandler
  !verbose pop
!macroend

/**
 * Cleans up the registry for a file handler when the passed in value equals
 * the default value for the file handler. For HKCU the registry key is deleted
 * and for HKLM the default value is deleted.
 *
 * @param   _HANDLER_NAME
 *          The registry name for the handler.
 * @param   _DEFAULT_VALUE
 *          The value to check for against the handler's default value.
 *
 * $R6 = stores the long path to $INSTDIR
 * $R7 = _DEFAULT_VALUE
 * $R9 = _HANDLER_NAME
 */
!macro RegCleanFileHandler

  !ifndef ${_MOZFUNC_UN}RegCleanFileHandler
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetLongPath
    !insertmacro ${_MOZFUNC_UN_TMP}GetParent
    !insertmacro ${_MOZFUNC_UN_TMP}GetPathFromString
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}RegCleanFileHandler "!insertmacro ${_MOZFUNC_UN}RegCleanFileHandlerCall"

    Function ${_MOZFUNC_UN}RegCleanFileHandler
      Exch $R9
      Exch 1
      Exch $R8
      Push $R7

      ReadRegStr $R7 HKCU "Software\Classes\$R9" ""
      StrCmp "$R7" "$R8" +1 +2
      DeleteRegKey HKCU "Software\Classes\$R9"

      ReadRegStr $R7 HKLM "Software\Classes\$R9" ""
      StrCmp "$R7" "$R8" +1 +2
      DeleteRegValue HKLM "Software\Classes\$R9" ""

      ClearErrors

      Pop $R7
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro RegCleanFileHandlerCall _HANDLER_NAME _DEFAULT_VALUE
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_DEFAULT_VALUE}"
  Push "${_HANDLER_NAME}"
  Call RegCleanFileHandler
  !verbose pop
!macroend

!macro un.RegCleanFileHandlerCall _HANDLER_NAME _DEFAULT_VALUE
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_DEFAULT_VALUE}"
  Push "${_HANDLER_NAME}"
  Call un.RegCleanFileHandler
  !verbose pop
!macroend

!macro un.RegCleanFileHandler
  !ifndef un.RegCleanFileHandler
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro RegCleanFileHandler

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Checks if a handler's open command points to this installation directory.
 *
 * @param   _HANDLER_NAME
 *          The registry name for the handler.
 * @param   _RESULT
 *          true if it is the handler's open command points to this
 *          installation directory and false if it does not.
 *
 * $R7 = stores the value of the open command and the path macros return values
 * $R8 = stores the handler's registry key name
 * $R9 = _DEFAULT_VALUE and _RESULT
 */
!macro IsHandlerForInstallDir

  !ifndef ${_MOZFUNC_UN}IsHandlerForInstallDir
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}GetLongPath
    !insertmacro ${_MOZFUNC_UN_TMP}GetParent
    !insertmacro ${_MOZFUNC_UN_TMP}GetPathFromString
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}IsHandlerForInstallDir "!insertmacro ${_MOZFUNC_UN}IsHandlerForInstallDirCall"

    Function ${_MOZFUNC_UN}IsHandlerForInstallDir
      Exch $R9
      Push $R8
      Push $R7

      StrCpy $R8 "$R9"
      StrCpy $R9 "false"
      ReadRegStr $R7 HKCR "$R8\shell\open\command" ""
      StrCmp "$R7" "" end

      ${GetPathFromString} "$R7" $R7
      ${GetParent} "$R7" $R7
      ${GetLongPath} "$R7" $R7
      StrCmp "$R7" "$INSTDIR" +1 end
      StrCpy $R9 "true"

      end:
      ClearErrors

      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro IsHandlerForInstallDirCall _HANDLER_NAME _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_HANDLER_NAME}"
  Call IsHandlerForInstallDir
  Pop "${_RESULT}"
  !verbose pop
!macroend

!macro un.IsHandlerForInstallDirCall _HANDLER_NAME _RESULT
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_HANDLER_NAME}"
  Call un.IsHandlerForInstallDir
  Pop "${_RESULT}"
  !verbose pop
!macroend

!macro un.IsHandlerForInstallDir
  !ifndef un.IsHandlerForInstallDir
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro IsHandlerForInstallDir

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * If present removes the VirtualStore directory for this installation. Uses the
 * program files directory path and the current install location to determine
 * the sub-directory in the VirtualStore directory.
 */
!macro CleanVirtualStore

  !ifndef ${_MOZFUNC_UN}CleanVirtualStore
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}CleanVirtualStore "!insertmacro ${_MOZFUNC_UN}CleanVirtualStoreCall"

    Function ${_MOZFUNC_UN}CleanVirtualStore
      Push $R9
      Push $R8
      Push $R7

      StrLen $R9 "$INSTDIR"

      ; Get the installation's directory name including the preceding slash
      start:
      IntOp $R8 $R8 - 1
      IntCmp $R8 -$R9 end end +1
      StrCpy $R7 "$INSTDIR" 1 $R8
      StrCmp $R7 "\" +1 start

      StrCpy $R9 "$INSTDIR" "" $R8

      ClearErrors
      GetFullPathName $R8 "$PROGRAMFILES$R9"
      IfErrors end
      GetFullPathName $R7 "$INSTDIR"

      ; Compare the installation's directory path with the path created by
      ; concatenating the installation's directory name and the path to the
      ; program files directory.
      StrCmp "$R7" "$R8" +1 end

      StrCpy $R8 "$PROGRAMFILES" "" 2 ; Remove the drive letter and colon
      StrCpy $R7 "$PROFILE\AppData\Local\VirtualStore$R8$R9"

      IfFileExists "$R7" 0 end
      RmDir /r "$R7"

      end:
      ClearErrors

      Pop $R7
      Pop $R8
      Pop $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CleanVirtualStoreCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call CleanVirtualStore
  !verbose pop
!macroend

!macro un.CleanVirtualStoreCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.CleanVirtualStore
  !verbose pop
!macroend

!macro un.CleanVirtualStore
  !ifndef un.CleanVirtualStore
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro CleanVirtualStore

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Deletes all relative profiles specified in an application's profiles.ini and
 * performs various other cleanup.
 *
 * @param   _REL_PROFILE_PATH
 *          The relative path to the profile directory.
 *
 * $R6 = value of IsRelative read from profiles.ini
 * $R7 = value of Path to profile read from profiles.ini
 * $R8 = counter for reading profiles (e.g. Profile0, Profile1, etc.)
 * $R9 = _REL_PROFILE_PATH
 */
!macro DeleteRelativeProfiles

  !ifndef ${_MOZFUNC_UN}DeleteRelativeProfiles
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}WordReplace
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}DeleteRelativeProfiles "!insertmacro ${_MOZFUNC_UN}DeleteRelativeProfilesCall"

    Function ${_MOZFUNC_UN}DeleteRelativeProfiles
      Exch $R9
      Push $R8
      Push $R7
      Push $R6

      SetShellVarContext current
      StrCpy $R8 -1

      loop:
      IntOp $R8 $R8 + 1  ; Increment the counter.
      ReadINIStr $R7 "$APPDATA\$R9\profiles.ini" "Profile$R8" "Path"
      IfErrors end +1

      ; Only remove relative profiles
      ReadINIStr $R6 "$APPDATA\$R9\profiles.ini" "Profile$R8" "IsRelative"
      StrCmp "$R6" "1" +1 loop

      ; Relative paths in profiles.ini use / as a separator
      ${${_MOZFUNC_UN}WordReplace} "$R7" "/" "\" "+" $R7

      IfFileExists "$LOCALAPPDATA\$R9\$R7" +1 +2
      RmDir /r "$LOCALAPPDATA\$R9\$R7"
      IfFileExists "$APPDATA\$R9\$R7" +1 +2
      RmDir /r "$APPDATA\$R9\$R7"
      GoTo loop

      end:
      ; Remove profiles directory under LOCALAPPDATA (e.g. cache, etc.) since
      ; they are at times abandoned.
      RmDir /r "$LOCALAPPDATA\$R9\Profiles"
      RmDir /r "$APPDATA\$R9\Crash Reports"
      Delete "$APPDATA\$R9\profiles.ini"
      Delete "$APPDATA\$R9\console.log"
      Delete "$APPDATA\$R9\pluginreg.dat"

      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro DeleteRelativeProfilesCall _REL_PROFILE_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_REL_PROFILE_PATH}"
  Call DeleteRelativeProfiles
  !verbose pop
!macroend

!macro un.DeleteRelativeProfilesCall _REL_PROFILE_PATH
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_REL_PROFILE_PATH}"
  Call un.DeleteRelativeProfiles
  !verbose pop
!macroend

!macro un.DeleteRelativeProfiles
  !ifndef un.DeleteRelativeProfiles
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro DeleteRelativeProfiles

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend


################################################################################
# Macros for parsing and updating the uninstall.log and removed-files.log

/**
 * Updates the uninstall.log with new files added by software update.
 *
 * When modifying this macro be aware that LineFind uses all registers except
 * $R0-$R3 so be cautious. Callers of this macro are not affected.
 */
!macro UpdateUninstallLog

  !ifndef UpdateUninstallLog
    !insertmacro FileJoin
    !insertmacro LineFind
    !insertmacro TextCompareNoDetails
    !insertmacro TrimNewLines

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define UpdateUninstallLog "!insertmacro UpdateUninstallLogCall"

    Function UpdateUninstallLog
      Push $R3
      Push $R2
      Push $R1
      Push $R0

      ClearErrors

      GetFullPathName $R3 "$INSTDIR\uninstall"
      IfFileExists "$R3\uninstall.update" +2 0
      Return

      ${LineFind} "$R3\uninstall.update" "" "1:-1" "CleanupUpdateLog"

      GetTempFileName $R2 "$R3"
      FileOpen $R1 $R2 w
      ${TextCompareNoDetails} "$R3\uninstall.update" "$R3\uninstall.log" "SlowDiff" "CreateUpdateDiff"
      FileClose $R1

      IfErrors +2 0
      ${FileJoin} "$R3\uninstall.log" "$R2" "$R3\uninstall.log"

      ${DeleteFile} "$R2"

      ClearErrors

      Pop $R0
      Pop $R1
      Pop $R2
      Pop $R3
    FunctionEnd

    ; This callback MUST use labels vs. relative line numbers.
    Function CleanupUpdateLog
      StrCpy $R2 "$R9" 12
      StrCmp "$R2" "EXECUTE ADD " +1 skip
      StrCpy $R9 "$R9" "" 12

      Push $R6
      Push $R5
      Push $R4
      StrCpy $R4 ""         ; Initialize to an empty string.
      StrCpy $R6 -1         ; Set the counter to -1 so it will start at 0.

      loop:
      IntOp $R6 $R6 + 1     ; Increment the counter.
      StrCpy $R5 $R9 1 $R6  ; Starting from the counter copy the next char.
      StrCmp $R5 "" copy    ; Are there no more chars?
      StrCmp $R5 "/" +1 +2  ; Is the char a /?
      StrCpy $R5 "\"        ; Replace the char with a \.

      StrCpy $R4 "$R4$R5"
      GoTo loop

      copy:
      StrCpy $R9 "File: \$R4"
      Pop $R6
      Pop $R5
      Pop $R4
      GoTo end

      skip:
      StrCpy $0 "SkipWrite"

      end:
      Push $0
    FunctionEnd

    Function CreateUpdateDiff
      ${TrimNewLines} "$9" $9
      StrCmp $9 "" +2 +1
      FileWrite $R1 "$9$\r$\n"

      Push 0
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro UpdateUninstallLogCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call UpdateUninstallLog
  !verbose pop
!macroend

/**
 * Updates the uninstall.log with entries from uninstall.bak. The uninstall.bak
 * is the uninstall.log renamed to uninstall.bak at the beginning of the
 * installation
 *
 * When modifying this macro be aware that LineFind uses all registers except
 * $R0-$R3 so be cautious. Callers of this macro are not affected.
 */
!macro UpdateFromPreviousLog

  !ifndef UpdateFromPreviousLog
    !insertmacro FileJoin
    !insertmacro GetTime
    !insertmacro TextCompareNoDetails
    !insertmacro TrimNewLines

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define UpdateFromPreviousLog "!insertmacro UpdateFromPreviousLogCall"

    Function UpdateFromPreviousLog
      Push $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2
      Push $R1
      Push $R0
      Push $9

      ; Diff and add missing entries from the previous file log if it exists
      IfFileExists "$INSTDIR\uninstall\uninstall.bak" +1 end
      StrCpy $R0 "$INSTDIR\uninstall\uninstall.log"
      StrCpy $R1 "$INSTDIR\uninstall\uninstall.bak"
      GetTempFileName $R2
      FileOpen $R3 $R2 w
      ${TextCompareNoDetails} "$R1" "$R0" "SlowDiff" "UpdateFromPreviousLog_AddToLog"
      FileClose $R3
      IfErrors +2
      ${FileJoin} "$INSTDIR\uninstall\uninstall.log" "$R2" "$INSTDIR\uninstall\uninstall.log"

      ${DeleteFile} "$INSTDIR\uninstall\uninstall.bak"
      ${DeleteFile} "$R2"

      end:

      Pop $9
      Pop $R0
      Pop $R1
      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Push $R9
    FunctionEnd

    Function UpdateFromPreviousLog_AddToLog
      ${TrimNewLines} "$9" $9
      StrCmp $9 "" end +1
      FileWrite $R3 "$9$\r$\n"
      ${LogMsg} "Added To Uninstall Log: $9"

      end:
      Push 0
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro UpdateFromPreviousLogCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call UpdateFromPreviousLog
  !verbose pop
!macroend

/**
 * Parses the removed-files.log to remove files, and directories prior to
 * installing.
 *
 * When modifying this macro be aware that LineFind uses all registers except
 * $R0-$R3 so be cautious. Callers of this macro are not affected.
 */
!macro ParseRemovedFilesLog

  !ifndef ParseRemovedFilesLog
    !insertmacro LineFind
    !insertmacro TrimNewLines

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ParseRemovedFilesLog "!insertmacro ParseRemovedFilesLogCall"

    Function ParseRemovedFilesLog
      Push $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2
      Push $R1
      Push $R0

      IfFileExists "$EXEDIR\removed-files.log" +1 end
      ${LogHeader} "Removing Obsolete Files and Directories"
      ${LineFind} "$EXEDIR\removed-files.log" "/NUL" "1:-1" "ParseRemovedFilesLog_RemoveFile"
      ${LineFind} "$EXEDIR\removed-files.log" "/NUL" "1:-1" "ParseRemovedFilesLog_RemoveDir"

      end:

      Pop $R0
      Pop $R1
      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Pop $R9
    FunctionEnd

    Function ParseRemovedFilesLog_RemoveFile
      ${TrimNewLines} "$R9" $R9
      StrCpy $R1 "$R9" 5

      StrCmp $R1 "File:" +1 end
      StrCpy $R9 "$R9" "" 6
      IfFileExists "$INSTDIR$R9" +1 end
      ClearErrors
      Delete "$INSTDIR$R9"
      IfErrors +1 +3
      ${LogMsg} "** ERROR Deleting File: $INSTDIR$R9 **"
      GoTo end

      ${LogMsg} "Deleted File: $INSTDIR$R9"

      end:
      ClearErrors

      Push 0
    FunctionEnd

    ; The xpinstall based installer removed directories even when they aren't
    ; empty so this does as well.
    Function ParseRemovedFilesLog_RemoveDir
      ${TrimNewLines} "$R9" $R9
      StrCpy $R1 "$R9" 4
      StrCmp "$R1" "Dir:" +1 end
      StrCpy $R9 "$R9" "" 5
      StrCpy $R1 "$R9" "" -1

      StrCmp "$R1" "\" +1 +2
      StrCpy $R9 "$R9" -1

      IfFileExists "$INSTDIR$R9" +1 end
      ClearErrors
      RmDir /r "$INSTDIR$R9"
      IfErrors +1 +3
      ${LogMsg} "** ERROR Removing Directory: $INSTDIR$R9 **"
      GoTo end

      ${LogMsg} "Removed Directory: $INSTDIR$R9"

      end:
      ClearErrors

      Push 0
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro ParseRemovedFilesLogCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call ParseRemovedFilesLog
  !verbose pop
!macroend

/**
 * Copies files from a source directory to a destination directory with logging
 * to the uninstall.log. If any destination files are in use a reboot will be
 * necessary to complete the installation and the reboot flag (see IfRebootFlag
 * in the NSIS documentation).
 *
 * @param   _PATH_TO_SOURCE
 *          Source path to copy the files from. This must not end with a \.
 *
 * @param   _PATH_TO_DESTINATION
 *          Destination path to copy the files to. This must not end with a \.
 *
 * @param   _PREFIX_ERROR_CREATEDIR
 *          Prefix for the directory creation error message. The directory path
 *          will be inserted below this string.
 *
 * @param   _SUFFIX_ERROR_CREATEDIR
 *          Prefix for the directory creation error message. The directory path
 *          will be inserted above this string.
 *
 * $0  = destination file's parent directory used in the create_dir label
 * $R0 = copied value from $R6 (e.g. _PATH_TO_SOURCE)
 * $R1 = copied value from $R7 (e.g. _PATH_TO_DESTINATION)
 * $R2 = string length of the path to source
 * $R3 = relative path from the path to source
 * $R4 = copied value from $R8 (e.g. _PREFIX_ERROR_CREATEDIR)
 * $R5 = copied value from $R9 (e.g. _SUFFIX_ERROR_CREATEDIR)
 * note: the LocateNoDetails macro uses these registers so we copy the values
 *       to other registers.
 * $R6 = initially _PATH_TO_SOURCE and then set to "size" ($R6="" if directory,
 *       $R6="0" if file with /S=)"path\name" in callback
 * $R7 = initially _PATH_TO_DESTINATION and then set to "name" in callback
 * $R8 = initially _PREFIX_ERROR_CREATEDIR and then set to "path" in callback
 * $R9 = initially _SUFFIX_ERROR_CREATEDIR and then set to "path\name" in
 *       callback
 */
!macro CopyFilesFromDir

  !ifndef CopyFilesFromDir
    !insertmacro LocateNoDetails
    !insertmacro OnEndCommon
    !insertmacro WordReplace

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define CopyFilesFromDir "!insertmacro CopyFilesFromDirCall"

    Function CopyFilesFromDir
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Exch 3
      Exch $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2
      Push $R1
      Push $R0
      Push $0

      StrCpy $R0 "$R6"
      StrCpy $R1 "$R7"
      StrCpy $R4 "$R8"
      StrCpy $R5 "$R9"

      StrLen $R2 $R0

      ${LocateNoDetails} "$R0" "/L=FD" "CopyFileCallback"

      Pop $0
      Pop $R0
      Pop $R1
      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Exch $R6
      Exch 3
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    Function CopyFileCallback
      StrCpy $R3 $R8 "" $R2 ; $R3 always begins with a \.

      retry:
      ClearErrors
      StrCmp $R6 "" +1 copy_file
      IfFileExists "$R1$R3\$R7" end +1
      StrCpy $0 "$R1$R3\$R7"

      create_dir:
      ClearErrors
      CreateDirectory "$0"
      IfFileExists "$0" +1 err_create_dir  ; protect against looping.
      ${LogMsg}  "Created Directory: $0"
      StrCmp $R6 "" end copy_file

      err_create_dir:
      ${LogMsg}  "** ERROR Creating Directory: $0 **"
      MessageBox MB_RETRYCANCEL|MB_ICONQUESTION "$R4$\r$\n$\r$\n$0$\r$\n$\r$\n$R5" IDRETRY retry
      ${OnEndCommon}
      Quit

      copy_file:
      StrCpy $0 "$R1$R3"
      IfFileExists "$0" +1 create_dir

      ClearErrors
      ${DeleteFile} "$R1$R3\$R7"
      IfErrors +1 dest_clear
      ClearErrors
      Rename "$R1$R3\$R7" "$R1$R3\$R7.moz-delete"
      IfErrors +1 reboot_delete

      ; file will replace destination file on reboot
      Rename "$R9" "$R9.moz-upgrade"
      CopyFiles /SILENT "$R9.moz-upgrade" "$R1$R3"
      Rename /REBOOTOK "$R1$R3\$R7.moz-upgrade" "$R1$R3\$R7"
      ${LogMsg} "Copied File: $R1$R3\$R7.moz-upgrade"
      ${LogMsg} "Delayed Install File (Reboot Required): $R1$R3\$R7"
      GoTo log_uninstall

      ; file will be deleted on reboot
      reboot_delete:
      CopyFiles /SILENT $R9 "$R1$R3"
      Delete /REBOOTOK "$R1$R3\$R7.moz-delete"
      ${LogMsg} "Installed File: $R1$R3\$R7"
      ${LogMsg} "Delayed Delete File (Reboot Required): $R1$R3\$R7.moz-delete"
      GoTo log_uninstall

      ; destination file doesn't exist - coast is clear
      dest_clear:
      CopyFiles /SILENT $R9 "$R1$R3"
      ${LogMsg} "Installed File: $R1$R3\$R7"

      log_uninstall:
      ; If the file is installed into the installation directory remove the
      ; installation directory's path from the file path when writing to the
      ; uninstall.log so it will be a relative path. This allows the same
      ; helper.exe to be used with zip builds if we supply an uninstall.log.
      ${WordReplace} "$R1$R3\$R7" "$INSTDIR" "" "+" $R3
      ${LogUninstall} "File: $R3"

      end:
      Push 0
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CopyFilesFromDirCall _PATH_TO_SOURCE _PATH_TO_DESTINATION \
                            _PREFIX_ERROR_CREATEDIR _SUFFIX_ERROR_CREATEDIR
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_PATH_TO_SOURCE}"
  Push "${_PATH_TO_DESTINATION}"
  Push "${_PREFIX_ERROR_CREATEDIR}"
  Push "${_SUFFIX_ERROR_CREATEDIR}"
  Call CopyFilesFromDir
  !verbose pop
!macroend

/**
 * Parses the uninstall.log to unregister dll's, remove files, and remove
 * empty directories for this installation.
 *
 * When modifying this macro be aware that LineFind uses all registers except
 * $R0-$R3 so be cautious. Callers of this macro are not affected.
 */
!macro un.ParseUninstallLog

  !ifndef un.ParseUninstallLog
    !insertmacro un.LineFind
    !insertmacro un.TrimNewLines

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define un.ParseUninstallLog "!insertmacro un.ParseUninstallLogCall"

    Function un.ParseUninstallLog
      Push $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2
      Push $R1
      Push $R0
      Push $TmpVal

      IfFileExists "$INSTDIR\uninstall\uninstall.log" +1 end

      ; Copy the uninstall log file to a temporary file
      GetTempFileName $TmpVal
      CopyFiles /SILENT /FILESONLY "$INSTDIR\uninstall\uninstall.log" "$TmpVal"

      ; Unregister DLL's
      ${un.LineFind} "$TmpVal" "/NUL" "1:-1" "un.UnRegDLLsCallback"

      ; Delete files
      ${un.LineFind} "$TmpVal" "/NUL" "1:-1" "un.RemoveFilesCallback"

      ; Remove empty directories
      ${un.LineFind} "$TmpVal" "/NUL" "1:-1" "un.RemoveDirsCallback"

      ; Delete the temporary uninstall log file
      Delete /REBOOTOK "$TmpVal"

      end:

      Pop $TmpVal
      Pop $R0
      Pop $R1
      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Pop $R9
    FunctionEnd

    Function un.RemoveFilesCallback
      ${un.TrimNewLines} "$R9" $R9
      StrCpy $R1 "$R9" 5

      StrCmp "$R1" "File:" +1 end
      StrCpy $R9 "$R9" "" 6
      StrCpy $R0 "$R9" 1

      StrCpy $R1 "$INSTDIR$R9"
      StrCmp "$R0" "\" +2 +1
      StrCpy $R1 "$R9"

      IfFileExists "$R1" +1 end
      Delete "$R1"
      IfErrors +1 end
      ClearErrors
      Rename "$R1" "$R1.moz-delete"
      IfErrors +1 +3
      Delete /REBOOTOK "$R1"
      GoTo end

      Delete /REBOOTOK "$R1.moz-delete"

      end:
      ClearErrors

      Push 0
    FunctionEnd

    Function un.UnRegDLLsCallback
      ${un.TrimNewLines} "$R9" $R9
      StrCpy $R1 "$R9" 7

      StrCmp $R1 "DLLReg:" +1 end
      StrCpy $R9 "$R9" "" 8
      StrCpy $R0 "$R9" 1

      StrCpy $R1 "$INSTDIR$R9"
      StrCmp $R0 "\" +2 +1
      StrCpy $R1 "$R9"

      UnRegDLL $R1

      end:
      ClearErrors

      Push 0
    FunctionEnd

    ; Using locate will leave file handles open to some of the directories
    ; which will prevent the deletion of these directories. This parses the
    ; uninstall.log and uses the file entries to find / remove empty
    ; directories.
    Function un.RemoveDirsCallback
      ${un.TrimNewLines} "$R9" $R9
      StrCpy $R0 "$R9" 5          ; Copy the first five chars
      StrCmp "$R0" "File:" +1 end

      StrCpy $R9 "$R9" "" 6       ; Copy string starting after the 6th char
      StrCpy $R0 "$R9" 1          ; Copy the first char

      StrCpy $R1 "$INSTDIR$R9"    ; Copy the install dir path and suffix it with the string
      StrCmp "$R0" "\" loop       ; If this is a relative path goto the loop
      StrCpy $R1 "$R9"            ; Already a full path so copy the string

      loop:
      ${un.GetParent} "$R1" $R1   ; Get the parent directory for the path
      StrCmp "$R1" "$INSTDIR" end ; If the directory is the install dir goto end

      ; We only try to remove empty directories but the Desktop, StartMenu, and
      ; QuickLaunch directories can be empty so guard against removing them.
      SetShellVarContext all          ; Set context to all users
      StrCmp "$R1" "$DESKTOP" end     ; All users desktop
      StrCmp "$R1" "$STARTMENU" end   ; All users start menu

      SetShellVarContext current      ; Set context to all users
      StrCmp "$R1" "$DESKTOP" end     ; Current user desktop
      StrCmp "$R1" "$STARTMENU" end   ; Current user start menu
      StrCmp "$R1" "$QUICKLAUNCH" end ; Current user quick launch

      IfFileExists "$R1" +1 +3  ; Only try to remove the dir if it exists
      ClearErrors
      RmDir "$R1"    ; Remove the dir
      IfErrors end   ; If we fail there is no use trying to remove its parent dir

      StrCmp "$R0" "\" loop end  ; Only loop when the path is relative to the install dir

      end:
      ClearErrors

      Push 0
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro un.ParseUninstallLogCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.ParseUninstallLog
  !verbose pop
!macroend


################################################################################
# Macros for custom branding

/**
 * Sets BrandFullName and / or BrandShortName to values provided in the specified
 * ini file and defaults to BrandShortName and BrandFullName as defined in
 * branding.nsi when the associated ini file entry is not specified.
 *
 * ini file format:
 * [Branding]
 * BrandFullName=Custom Full Name
 * BrandShortName=Custom Short Name
 *
 * @param   _PATH_TO_INI
 *          Path to the ini file.
 *
 * $R6 = return value from ReadINIStr
 * $R7 = stores BrandShortName
 * $R8 = stores BrandFullName
 * $R9 = _PATH_TO_INI
 */
!macro SetBrandNameVars

  !ifndef ${_MOZFUNC_UN}SetBrandNameVars
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}WordReplace
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    ; Prevent declaring vars twice when the SetBrandNameVars macro is
    ; inserted into both the installer and uninstaller.
    !ifndef SetBrandNameVars
      Var BrandFullName
      Var BrandFullNameDA
      Var BrandShortName
    !endif

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}SetBrandNameVars "!insertmacro ${_MOZFUNC_UN}SetBrandNameVarsCall"

    Function ${_MOZFUNC_UN}SetBrandNameVars
      Exch $R9
      Push $R8
      Push $R7
      Push $R6

      StrCpy $R8 "${BrandFullName}"
      StrCpy $R7 "${BrandShortName}"

      IfFileExists "$R9" +1 finish

      ClearErrors
      ReadINIStr $R6 $R9 "Branding" "BrandFullName"
      IfErrors +2 +1
      StrCpy $R8 "$R6"

      ClearErrors
      ReadINIStr $R6 $R9 "Branding" "BrandShortName"
      IfErrors +2 +1
      StrCpy $R7 "$R6"

      finish:
      StrCpy $BrandFullName "$R8"
      ${${_MOZFUNC_UN}WordReplace} "$R8" "&" "&&" "+" $R8
      StrCpy $BrandFullNameDA "$R8"
      StrCpy $BrandShortName "$R7"

      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro SetBrandNameVarsCall _PATH_TO_INI
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_PATH_TO_INI}"
  Call SetBrandNameVars
  !verbose pop
!macroend

!macro un.SetBrandNameVarsCall _PATH_TO_INI
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_PATH_TO_INI}"
  Call un.SetBrandNameVars
  !verbose pop
!macroend

!macro un.SetBrandNameVars
  !ifndef un.SetBrandNameVars
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro SetBrandNameVars

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Replaces the wizard's header image with the one specified.
 *
 * @param   _PATH_TO_IMAGE
 *          Fully qualified path to the bitmap to use for the header image.
 *
 * $R8 = hwnd for the control returned from GetDlgItem.
 * $R9 = _PATH_TO_IMAGE
 */
!macro ChangeMUIHeaderImage

  !ifndef ${_MOZFUNC_UN}ChangeMUIHeaderImage
    Var hHeaderBitmap

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}ChangeMUIHeaderImage "!insertmacro ${_MOZFUNC_UN}ChangeMUIHeaderImageCall"

    Function ${_MOZFUNC_UN}ChangeMUIHeaderImage
      Exch $R9
      Push $R8

      GetDlgItem $R8 $HWNDPARENT 1046
      System::Call 'user32::LoadImage(i 0, t "$R9", i 0, i 0, i 0, i 0x0010|0x2000) i.s'
      Pop $hHeaderBitmap
      SendMessage $R8 ${STM_SETIMAGE} 0 $hHeaderBitmap
      ; There is no way to specify a show function for a custom page so hide
      ; and then show the control to force the bitmap to redraw.
      ShowWindow $R8 ${SW_HIDE}
      ShowWindow $R8 ${SW_SHOW}

      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro ChangeMUIHeaderImageCall _PATH_TO_IMAGE
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_PATH_TO_IMAGE}"
  Call ChangeMUIHeaderImage
  !verbose pop
!macroend

!macro un.ChangeMUIHeaderImageCall _PATH_TO_IMAGE
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_PATH_TO_IMAGE}"
  Call un.ChangeMUIHeaderImage
  !verbose pop
!macroend

!macro un.ChangeMUIHeaderImage
  !ifndef un.ChangeMUIHeaderImage
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro ChangeMUIHeaderImage

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend


################################################################################
# User interface callback helper defines and macros

/* Install type defines */
!ifndef INSTALLTYPE_BASIC
  !define INSTALLTYPE_BASIC     1
!endif

!ifndef INSTALLTYPE_ADVANCED
  !define INSTALLTYPE_ADVANCED  2
!endif

!ifndef INSTALLTYPE_CUSTOM
  !define INSTALLTYPE_CUSTOM    4
!endif

/**
 * Checks whether to display the current page (e.g. if not performing a custom
 * install don't display the custom pages).
 */
!macro CheckCustomCommon

  !ifndef CheckCustomCommon
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define CheckCustomCommon "!insertmacro CheckCustomCommonCall"

    Function CheckCustomCommon

      ; Abort if not a custom install
      IntCmp $InstallType ${INSTALLTYPE_CUSTOM} +2 +1 +1
      Abort

    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro CheckCustomCommonCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call CheckCustomCommon
  !verbose pop
!macroend

/**
 * Unloads dll's and releases references when the installer and uninstaller
 * exit.
 */
!macro OnEndCommon

  !ifndef ${_MOZFUNC_UN}OnEndCommon
    !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
    !insertmacro ${_MOZFUNC_UN_TMP}UnloadUAC
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
    !undef _MOZFUNC_UN_TMP

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}OnEndCommon "!insertmacro ${_MOZFUNC_UN}OnEndCommonCall"

    Function ${_MOZFUNC_UN}OnEndCommon

      ${${_MOZFUNC_UN}UnloadUAC}
      StrCmp $hHeaderBitmap "" +3 +1
      System::Call "gdi32::DeleteObject(i s)" $hHeaderBitmap
      StrCpy $hHeaderBitmap ""

      System::Free 0

    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro OnEndCommonCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call OnEndCommon
  !verbose pop
!macroend

!macro un.OnEndCommonCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.OnEndCommon
  !verbose pop
!macroend

!macro un.OnEndCommon
  !ifndef un.OnEndCommon
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro OnEndCommon

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Called from the installer's .onInit function not to be confused with the
 * uninstaller's .onInit or the uninstaller's un.onInit functions.
 *
 * @param   _WARN_UNSUPPORTED_MSG
 *          Message displayed when the Windows version is not supported.
 *
 * $R5 = return value from the GetSize macro
 * $R6 = general string values, return value from GetTempFileName, return
 *       value from the GetSize macro
 * $R7 = full path to the configuration ini file
 * $R8 = return value from the GetParameters macro
 * $R9 = _WARN_UNSUPPORTED_MSG
 */
!macro InstallOnInitCommon

  !ifndef InstallOnInitCommon
    !insertmacro CloseApp
    !insertmacro ElevateUAC
    !insertmacro GetOptions
    !insertmacro GetParameters
    !insertmacro GetSize

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define InstallOnInitCommon "!insertmacro InstallOnInitCommonCall"

    Function InstallOnInitCommon
      Exch $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5

      !ifdef ___WINVER__NSH___
        ${Unless} ${AtLeastWin2000}
          MessageBox MB_OK|MB_ICONSTOP "$R9" IDOK
          ; Nothing initialized so no need to call OnEndCommon
          Quit
        ${EndUnless}
      !endif

      ${GetParameters} $R8

      ; Require elevation if the user can elevate
      ${ElevateUAC}

      ${If} $R8 != ""
        ClearErrors
        ${GetOptions} "$R8" "-ms" $R7
        ${If} ${Errors}
          ; Default install type
          StrCpy $InstallType ${INSTALLTYPE_BASIC}
          ; Support for specifying an installation configuration file.
          ClearErrors
          ${GetOptions} "$R8" "/INI=" $R7
          ${Unless} ${Errors}
            ; The configuration file must also exist
            ${If} ${FileExists} "$R7"
              SetSilent silent
              ReadINIStr $R8 $R7 "Install" "InstallDirectoryName"
              ${If} $R8 != ""
                StrCpy $INSTDIR "$PROGRAMFILES\$R8"
              ${Else}
                ReadINIStr $R8 $R7 "Install" "InstallDirectoryPath"
                ${If} $R8 != ""
                  StrCpy $INSTDIR "$R8"
                ${EndIf}
              ${EndIf}

              ${If} $INSTDIR == ""
                ; Check if there is an existing uninstall registry entry for this
                ; version of the application and if present install into that location
                StrCpy $R6 "Software\Microsoft\Windows\CurrentVersion\Uninstall\${BrandFullNameInternal} (${AppVersion})"
                ReadRegStr $R8 HKLM "$R6" "InstallLocation"
                ${If} $R8 == ""
                  StrCpy $INSTDIR "$PROGRAMFILES\${BrandFullName}"
                ${Else}
                  GetFullPathName $INSTDIR "$R8"
                  ${Unless} ${FileExists} "$INSTDIR"
                    StrCpy $INSTDIR "$PROGRAMFILES\${BrandFullName}"
                  ${EndUnless}
                ${EndIf}
              ${EndIf}

              ; Quit if we are unable to create the installation directory or we are
              ; unable to write to a file in the installation directory.
              ClearErrors
              ${If} ${FileExists} "$INSTDIR"
                GetTempFileName $R6 "$INSTDIR"
                FileOpen $R5 $R6 w
                FileWrite $R5 "Write Access Test"
                FileClose $R5
                Delete $R6
                ${If} ${Errors}
                  ; Nothing initialized so no need to call OnEndCommon
                  Quit
                ${EndIf}
              ${Else}
                CreateDirectory "$INSTDIR"
                ${If} ${Errors}
                  ; Nothing initialized so no need to call OnEndCommon
                  Quit
                ${EndIf}
              ${EndIf}

              ReadINIStr $R8 $R7 "Install" "CloseAppNoPrompt"
              ${If} $R8 == "true"
                ; Try to close the app if the exe is in use.
                ClearErrors
                ${If} ${FileExists} "$INSTDIR\${FileMainEXE}"
                  ${DeleteFile} "$INSTDIR\${FileMainEXE}"
                ${EndIf}
                ${If} ${Errors}
                  ClearErrors
                  ${CloseApp} "false" ""
                  ClearErrors
                  ${DeleteFile} "$INSTDIR\${FileMainEXE}"
                  ; If unsuccessful try one more time and if it still fails Quit
                  ${If} ${Errors}
                    ClearErrors
                    ${CloseApp} "false" ""
                    ClearErrors
                    ${DeleteFile} "$INSTDIR\${FileMainEXE}"
                    ${If} ${Errors}
                      ; Nothing initialized so no need to call OnEndCommon
                      Quit
                    ${EndIf}
                  ${EndIf}
                ${EndIf}
              ${EndIf}

              ReadINIStr $R8 $R7 "Install" "QuickLaunchShortcut"
              ${If} $R8 == "false"
                StrCpy $AddQuickLaunchSC "0"
              ${Else}
                StrCpy $AddQuickLaunchSC "1"
              ${EndIf}

              ReadINIStr $R8 $R7 "Install" "DesktopShortcut"
              ${If} $R8 == "false"
                StrCpy $AddDesktopSC "0"
              ${Else}
                StrCpy $AddDesktopSC "1"
              ${EndIf}

              ReadINIStr $R8 $R7 "Install" "StartMenuShortcuts"
              ${If} $R8 == "false"
                StrCpy $AddStartMenuSC "0"
              ${Else}
                StrCpy $AddStartMenuSC "1"
              ${EndIf}

              ReadINIStr $R8 $R7 "Install" "StartMenuDirectoryName"
              ${If} $R8 != ""
                StrCpy $StartMenuDir "$R8"
              ${EndIf}
            ${EndIf}
          ${EndUnless}
        ${Else}
          ; Support for the deprecated -ms command line argument. The new command
          ; line arguments are not supported when -ms is used.
          SetSilent silent
        ${EndIf}
      ${EndIf}
      ClearErrors

      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro InstallOnInitCommonCall _WARN_UNSUPPORTED_MSG
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push ${_WARN_UNSUPPORTED_MSG}
  Call InstallOnInitCommon
  !verbose pop
!macroend

/**
 * Called from the uninstaller's .onInit function not to be confused with the
 * installer's .onInit or the uninstaller's un.onInit functions.
 */
!macro UninstallOnInitCommon

  !ifndef UninstallOnInitCommon
    !insertmacro ElevateUAC
    !insertmacro GetLongPath
    !insertmacro GetOptions
    !insertmacro GetParameters
    !insertmacro UnloadUAC
    !insertmacro UpdateUninstallLog

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define UninstallOnInitCommon "!insertmacro UninstallOnInitCommonCall"

    Function UninstallOnInitCommon

      ; Prevent launching the application when a reboot is required and this
      ; executable is the main application executable
      IfFileExists "$EXEDIR\${FileMainEXE}.moz-upgrade" +1 +4
      MessageBox MB_YESNO "$(WARN_RESTART_REQUIRED_UPGRADE)" IDNO +2
      Reboot
      Quit ; Nothing initialized so no need to call OnEndCommon

      GetFullPathName $INSTDIR "$EXEDIR\.."
      ${GetLongPath} "$INSTDIR" $INSTDIR
      IfFileExists "$INSTDIR\${FileMainEXE}" +2 +1
      Quit ; Nothing initialized so no need to call OnEndCommon

      ; Prevent all operations (e.g. set as default, postupdate, etc.) when a
      ; reboot is required and the executable launched is helper.exe
      IfFileExists "$INSTDIR\${FileMainEXE}.moz-upgrade" +1 +4
      MessageBox MB_YESNO "$(WARN_RESTART_REQUIRED_UPGRADE)" IDNO +2
      Reboot
      Quit ; Nothing initialized so no need to call OnEndCommon

      ${GetParameters} $R0

      StrCmp "$R0" "" continue +1

      ; Require elevation if the user can elevate
      ClearErrors
      ${GetOptions} "$R0" "/HideShortcuts" $R2
      IfErrors showshortcuts +1
      ${ElevateUAC}
      ${HideShortcuts}
      GoTo finish

      ; Require elevation if the user can elevate
      showshortcuts:
      ClearErrors
      ${GetOptions} "$R0" "/ShowShortcuts" $R2
      IfErrors defaultappuser +1
      ${ElevateUAC}
      ${ShowShortcuts}
      GoTo finish

      ; Require elevation if the the StartMenuInternet registry keys require
      ; updating and the user can elevate
      defaultappuser:
      ClearErrors
      ${GetOptions} "$R0" "/SetAsDefaultAppUser" $R2
      IfErrors defaultappglobal +1
      ${SetAsDefaultAppUser}
      GoTo finish

      ; Require elevation if the user can elevate
      defaultappglobal:
      ClearErrors
      ${GetOptions} "$R0" "/SetAsDefaultAppGlobal" $R2
      IfErrors postupdate +1
      ${ElevateUAC}
      ${SetAsDefaultAppGlobal}
      GoTo finish

      ; Do not attempt to elevate. The application launching this executable is
      ; responsible for elevation if it is required.
      postupdate:
      ${WordReplace} "$R0" "$\"" "" "+" $R0
      ClearErrors
      ${GetOptions} "$R0" "/PostUpdate" $R2
      IfErrors continue +1
      ; If the uninstall.log does not exist don't perform post update
      ; operations. This prevents updating the registry for zip builds.
      IfFileExists "$EXEDIR\uninstall.log" +1 finish
      ${PostUpdate}
      ClearErrors
      ${GetOptions} "$R0" "/UninstallLog=" $R2
      IfErrors updateuninstalllog +1
      StrCmp "$R2" "" finish +1
      GetFullPathName $R3 "$R2"
      IfFileExists "$R3" +1 finish
      Delete "$INSTDIR\uninstall\*wizard*"
      Delete "$INSTDIR\uninstall\uninstall.log"
      CopyFiles /SILENT /FILESONLY "$R3" "$INSTDIR\uninstall\"
      ${GetParent} "$R3" $R4
      Delete "$R3"
      RmDir "$R4"
      GoTo finish

      ; Do not attempt to elevate. The application launching this executable is
      ; responsible for elevation if it is required.
      updateuninstalllog:
      ${UpdateUninstallLog}

      finish:
      ${UnloadUAC}
      System::Call "shell32::SHChangeNotify(i, i, i, i) v (0x08000000, 0, 0, 0)"
      Quit ; Nothing initialized so no need to call OnEndCommon

      continue:

      ; If the uninstall.log does not exist don't perform uninstall
      ; operations. This prevents running the uninstaller for zip builds.
      IfFileExists "$INSTDIR\uninstall\uninstall.log" +2 +1
      Quit ; Nothing initialized so no need to call OnEndCommon

      ; Require elevation if the user can elevate
      ${ElevateUAC}

      ; If we made it this far then this installer is being used as an uninstaller.
      WriteUninstaller "$EXEDIR\uninstaller.exe"

      StrCpy $R1 "$\"$EXEDIR\uninstaller.exe$\""
      StrCmp $R0 "/S" +1 +2
      StrCpy $R1 "$\"$EXEDIR\uninstaller.exe$\" /S"

      ; When the uninstaller is launched it copies itself to the temp directory
      ; so it won't be in use so it can delete itself.
      ExecWait $R1
      ${DeleteFile} "$EXEDIR\uninstaller.exe"
      ${UnloadUAC}
      SetErrorLevel 0
      Quit ; Nothing initialized so no need to call OnEndCommon

    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro UninstallOnInitCommonCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call UninstallOnInitCommon
  !verbose pop
!macroend

/**
 * Called from the MUI preDirectory function
 *
 * $R9 = returned value from GetSingleInstallPath, CheckDiskSpace, and
 *       CanWriteToInstallDir macros
 */
!macro PreDirectoryCommon

  !ifndef PreDirectoryCommon
    !insertmacro CanWriteToInstallDir
    !insertmacro CheckDiskSpace
    !insertmacro GetLongPath

!ifndef NO_INSTDIR_FROM_REG
    !insertmacro GetSingleInstallPath
!endif

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define PreDirectoryCommon "!insertmacro PreDirectoryCommonCall"

    Function PreDirectoryCommon
      Push $R9

!ifndef NO_INSTDIR_FROM_REG
      SetShellVarContext all      ; Set SHCTX to HKLM
      ${GetSingleInstallPath} "Software\Mozilla\${BrandFullNameInternal}" $R9

      StrCmp "$R9" "false" +1 fix_install_dir

      SetShellVarContext current  ; Set SHCTX to HKCU
      ${GetSingleInstallPath} "Software\Mozilla\${BrandFullNameInternal}" $R9

      fix_install_dir:
      StrCmp "$R9" "false" +2 +1
      StrCpy $INSTDIR "$R9"
!endif

      IfFileExists "$INSTDIR" +1 check_install_dir

      ; Always display the long path if the path already exists.
      ${GetLongPath} "$INSTDIR" $INSTDIR

      ; The call to GetLongPath returns a long path without a trailing
      ; back-slash. Append a \ to the path to prevent the directory
      ; name from being appended when using the NSIS create new folder.
      ; http://www.nullsoft.com/free/nsis/makensis.htm#InstallDir
      StrCpy $INSTDIR "$INSTDIR\"

      check_install_dir:
      IntCmp $InstallType ${INSTALLTYPE_CUSTOM} end +1 +1
      ${CheckDiskSpace} $R9
      StrCmp $R9 "false" end +1
      ${CanWriteToInstallDir} $R9
      StrCmp $R9 "false" end +1
      Abort

      end:

      Pop $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro PreDirectoryCommonCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call PreDirectoryCommon
  !verbose pop
!macroend

/**
 * Called from the MUI leaveDirectory function
 *
 * @param   _WARN_DISK_SPACE
 *          Message displayed when there isn't enough disk space to perform the
 *          installation.
 * @param   _WARN_WRITE_ACCESS
 *          Message displayed when the installer does not have write access to
 *          $INSTDIR.
 *
 * $R7 = returned value from CheckDiskSpace and CanWriteToInstallDir macros
 * $R8 = _WARN_DISK_SPACE
 * $R9 = _WARN_WRITE_ACCESS
 */
!macro LeaveDirectoryCommon

  !ifndef LeaveDirectoryCommon
    !insertmacro CheckDiskSpace
    !insertmacro CanWriteToInstallDir

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define LeaveDirectoryCommon "!insertmacro LeaveDirectoryCommonCall"

    Function LeaveDirectoryCommon
      Exch $R9
      Exch 1
      Exch $R8
      Push $R7

      ${CheckDiskSpace} $R7
      StrCmp $R7 "false" +1 +3
      MessageBox MB_OK "$R8"
      Abort

      ${CanWriteToInstallDir} $R7
      StrCmp $R7 "false" +1 +3
      MessageBox MB_OK "$R9"
      Abort

      Pop $R7
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro LeaveDirectoryCommonCall _WARN_DISK_SPACE _WARN_WRITE_ACCESS
  !verbose push
  Push "${_WARN_DISK_SPACE}"
  Push "${_WARN_WRITE_ACCESS}"
  !verbose ${_MOZFUNC_VERBOSE}
  Call LeaveDirectoryCommon
  !verbose pop
!macroend


################################################################################
# Install Section common macros.

/**
 * Performs common cleanup operations prior to the actual installation.
 * This macro should be called first when installation starts.
 */
!macro InstallStartCleanupCommon

  !ifndef InstallStartCleanupCommon
    !insertmacro CleanVirtualStore
    !insertmacro EndUninstallLog
    !insertmacro ParseRemovedFilesLog
    !insertmacro UpdateFromPreviousLog

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define InstallStartCleanupCommon "!insertmacro InstallStartCleanupCommonCall"

    Function InstallStartCleanupCommon

      ; Remove files not removed by parsing the removed-files.log
      Delete "$INSTDIR\install_wizard.log"
      Delete "$INSTDIR\install_status.log"

      RmDir /r "$INSTDIR\updates"
      Delete "$INSTDIR\updates.xml"
      Delete "$INSTDIR\active-update.xml"

      RmDir /r "$INSTDIR\distribution"

      ; Remove files from the uninstall directory.
      IfFileExists "$INSTDIR\uninstall" +1 +7
      Delete "$INSTDIR\uninstall\*wizard*"
      Delete "$INSTDIR\uninstall\uninstall.ini"
      Delete "$INSTDIR\uninstall\cleanup.log"
      Delete "$INSTDIR\uninstall\uninstall.update"
      IfFileExists "$INSTDIR\uninstall\uninstall.log" +1 +2
      Rename "$INSTDIR\uninstall\uninstall.log" "$INSTDIR\uninstall\uninstall.bak"

      ; Since we write to the uninstall.log in this directory during the
      ; installation create the directory if it doesn't already exist.
      IfFileExists "$INSTDIR\uninstall" +2 +1
      CreateDirectory "$INSTDIR\uninstall"

      ; Remove files that may be left behind by the application in the
      ; VirtualStore directory.
      ${CleanVirtualStore}

      ; Remove the files and directories in the removed-files.log
      ${ParseRemovedFilesLog}
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro InstallStartCleanupCommonCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call InstallStartCleanupCommon
  !verbose pop
!macroend

/**
 * Performs common cleanup operations after the actual installation.
 * This macro should be called last during the installation.
 */
!macro InstallEndCleanupCommon

  !ifndef InstallEndCleanupCommon
    !insertmacro EndUninstallLog
    !insertmacro UpdateFromPreviousLog

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define InstallEndCleanupCommon "!insertmacro InstallEndCleanupCommonCall"

    Function InstallEndCleanupCommon

      ; Close the file handle to the uninstall.log
      ${EndUninstallLog}
      ${UpdateFromPreviousLog}

    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro InstallEndCleanupCommonCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call InstallEndCleanupCommon
  !verbose pop
!macroend


################################################################################
# UAC Related Macros

/**
 * Provides UAC elevation support for Vista and above (requires the UAC plugin).
 *
 * $0 = return values from calls to the UAC plugin (always uses $0)
 * $R9 = return values from GetParameters and GetOptions macros
 */
!macro ElevateUAC

  !ifndef ${_MOZFUNC_UN}ElevateUAC
    !ifdef USE_UAC_PLUGIN
      !ifdef ___WINVER__NSH___
        !define _MOZFUNC_UN_TMP ${_MOZFUNC_UN}
        !insertmacro ${_MOZFUNC_UN_TMP}GetOptions
        !insertmacro ${_MOZFUNC_UN_TMP}GetParameters
        !undef _MOZFUNC_UN
        !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP}
        !undef _MOZFUNC_UN_TMP
      !endif
    !endif

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}ElevateUAC "!insertmacro ${_MOZFUNC_UN}ElevateUACCall"

    Function ${_MOZFUNC_UN}ElevateUAC
      ; USE_UAC_PLUGIN is temporary until Thunderbird has been updated to use the UAC plugin
      !ifdef USE_UAC_PLUGIN
        !ifdef ___WINVER__NSH___
          Push $R9
          Push $0

          ${If} ${AtLeastWinVista}
            UAC::IsAdmin
            ; If the user is not an admin already
            ${If} "$0" != "1"
              UAC::SupportsUAC
              ; If the system supports UAC
              ${If} "$0" == "1"
                UAC::GetElevationType
                ; If the user account has a split token
                ${If} "$0" == "3"
                  UAC::RunElevated
                  UAC::Unload
                  ; Nothing besides UAC initialized so no need to call OnEndCommon
                  Quit
                ${EndIf}
              ${EndIf}
            ${Else}
              ${GetParameters} $R9
              ${If} $R9 != ""
                ClearErrors
                ${GetOptions} "$R9" "/UAC:" $0
                ; If the command line contains /UAC then we need to initialize
                ; the UAC plugin to use UAC::ExecCodeSegment to execute code in
                ; the non-elevated context.
                ${Unless} ${Errors}
                  UAC::RunElevated 
                ${EndUnless}
              ${EndIf}
            ${EndIf}
          ${EndIf}

          Pop $0
          Pop $R9
        !endif
      !endif
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro ElevateUACCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call ElevateUAC
  !verbose pop
!macroend

!macro un.ElevateUACCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.ElevateUAC
  !verbose pop
!macroend

!macro un.ElevateUAC
  !ifndef un.ElevateUAC
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro ElevateUAC

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend

/**
 * Unloads the UAC plugin so the NSIS plugins can be removed when the installer
 * and uninstaller exit.
 *
 * $R9 = return values from GetParameters and GetOptions macros
 */
!macro UnloadUAC

  !ifndef ${_MOZFUNC_UN}UnloadUAC
    !ifdef USE_UAC_PLUGIN
      !ifdef ___WINVER__NSH___
        !define _MOZFUNC_UN_TMP_UnloadUAC ${_MOZFUNC_UN}
        !insertmacro ${_MOZFUNC_UN_TMP_UnloadUAC}GetOptions
        !insertmacro ${_MOZFUNC_UN_TMP_UnloadUAC}GetParameters
        !undef _MOZFUNC_UN
        !define _MOZFUNC_UN ${_MOZFUNC_UN_TMP_UnloadUAC}
        !undef _MOZFUNC_UN_TMP_UnloadUAC
      !endif
    !endif

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define ${_MOZFUNC_UN}UnloadUAC "!insertmacro ${_MOZFUNC_UN}UnloadUACCall"

    Function ${_MOZFUNC_UN}UnloadUAC
      !ifdef USE_UAC_PLUGIN
        !ifdef ___WINVER__NSH___
          Push $R9

          ${Unless} ${AtLeastWinVista}
            Return
          ${EndUnless}

          ClearErrors
          ${${_MOZFUNC_UN}GetParameters} $R9
          ${${_MOZFUNC_UN}GetOptions} "$R9" "/UAC:" $R9
          ; If the command line contains /UAC then we need to unload the UAC plugin
          IfErrors +2 +1
          UAC::Unload

          ClearErrors

          Pop $R9
        !endif
      !endif
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro UnloadUACCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call UnloadUAC
  !verbose pop
!macroend

!macro un.UnloadUACCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call un.UnloadUAC
  !verbose pop
!macroend

!macro un.UnloadUAC
  !ifndef un.UnloadUAC
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN "un."

    !insertmacro UnloadUAC

    !undef _MOZFUNC_UN
    !define _MOZFUNC_UN
    !verbose pop
  !endif
!macroend


################################################################################
# Macros for logging
#
# Since these are used by other macros they should be inserted first. All of
# these macros can be easily inserted using the _LoggingCommon macro.

/**
 * Adds all logging macros in the correct order in one fell swoop as well as
 * the vars for the install.log and uninstall.log file handles.
 */
!macro _LoggingCommon
  Var /GLOBAL fhInstallLog
  Var /GLOBAL fhUninstallLog
  !insertmacro StartInstallLog
  !insertmacro EndInstallLog
  !insertmacro StartUninstallLog
  !insertmacro EndUninstallLog
!macroend
!define _LoggingCommon "!insertmacro _LoggingCommon"

/**
 * Creates a file named install.log in the install directory (e.g. $INSTDIR)
 * and adds the installation started message to the install.log for this
 * installation. This also adds the fhInstallLog and fhUninstallLog vars used
 * for logging.
 *
 * $fhInstallLog   = filehandle for $INSTDIR\install.log
 *
 * @param   _APP_NAME
 *          Typically the BrandFullName
 * @param   _AB_CD
 *          The locale identifier
 * @param   _APP_VERSION
 *          The application version
 * @param   _GRE_VERSION
 *          The Gecko Runtime Engine version
 *
 * $R6 = _APP_NAME
 * $R7 = _AB_CD
 * $R8 = _APP_VERSION
 * $R9 = _GRE_VERSION
 */
!macro StartInstallLog

  !ifndef StartInstallLog
    !insertmacro GetTime

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define StartInstallLog "!insertmacro StartInstallLogCall"

    Function StartInstallLog
      Exch $R9
      Exch 1
      Exch $R8
      Exch 2
      Exch $R7
      Exch 3
      Exch $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2
      Push $R1
      Push $R0
      Push $9

      FileOpen $fhInstallLog "$INSTDIR\install.log" w

      ${GetTime} "" "L" $9 $R0 $R1 $R2 $R3 $R4 $R5
      FileWrite $fhInstallLog "$R6 Installation Started: $R1-$R0-$9 $R3:$R4:$R5"
      ${WriteLogSeparator}

      ${LogHeader} "Installation Details"
      ${LogMsg} "Install Dir: $INSTDIR"
      ${LogMsg} "Locale     : $R7"
      ${LogMsg} "App Version: $R8"
      ${LogMsg} "GRE Version: $R9"

      Pop $9
      Pop $R0
      Pop $R1
      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Exch $R6
      Exch 3
      Exch $R7
      Exch 2
      Exch $R8
      Exch 1
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro StartInstallLogCall _APP_NAME _AB_CD _APP_VERSION _GRE_VERSION
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_APP_NAME}"
  Push "${_AB_CD}"
  Push "${_APP_VERSION}"
  Push "${_GRE_VERSION}"
  Call StartInstallLog
  !verbose pop
!macroend

/**
 * Writes the installation finished message to the install.log and closes the
 * file handles to the install.log and uninstall.log
 *
 * @param   _APP_NAME
 *
 * $R9 = _APP_NAME
 */
!macro EndInstallLog

  !ifndef EndInstallLog
    !insertmacro GetTime

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define EndInstallLog "!insertmacro EndInstallLogCall"

    Function EndInstallLog
      Exch $R9
      Push $R8
      Push $R7
      Push $R6
      Push $R5
      Push $R4
      Push $R3
      Push $R2
      
      ${WriteLogSeparator}
      ${GetTime} "" "L" $R2 $R3 $R4 $R5 $R6 $R7 $R8
      FileWrite $fhInstallLog "$R9 Installation Finished: $R4-$R3-$R2 $R6:$R7:$R8$\r$\n"
      FileClose $fhInstallLog

      Pop $R2
      Pop $R3
      Pop $R4
      Pop $R5
      Pop $R6
      Pop $R7
      Pop $R8
      Exch $R9
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro EndInstallLogCall _APP_NAME
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Push "${_APP_NAME}"
  Call EndInstallLog
  !verbose pop
!macroend

/**
 * Opens the file handle to the uninstall.log.
 *
 * $fhUninstallLog = filehandle for $INSTDIR\uninstall\uninstall.log
 */
!macro StartUninstallLog

  !ifndef StartUninstallLog
    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define StartUninstallLog "!insertmacro StartUninstallLogCall"

    Function StartUninstallLog
      FileOpen $fhUninstallLog "$INSTDIR\uninstall\uninstall.log" w
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro StartUninstallLogCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call StartUninstallLog
  !verbose pop
!macroend

/**
 * Closes the file handle to the uninstall.log.
 */
!macro EndUninstallLog

  !ifndef EndUninstallLog

    !verbose push
    !verbose ${_MOZFUNC_VERBOSE}
    !define EndUninstallLog "!insertmacro EndUninstallLogCall"

    Function EndUninstallLog
      FileClose $fhUninstallLog
    FunctionEnd

    !verbose pop
  !endif
!macroend

!macro EndUninstallLogCall
  !verbose push
  !verbose ${_MOZFUNC_VERBOSE}
  Call EndUninstallLog
  !verbose pop
!macroend

/**
 * Adds a section header to the human readable log.
 *
 * @param   _HEADER
 *          The header text to write to the log.
 */
!macro LogHeader _HEADER
  ${WriteLogSeparator}
  FileWrite $fhInstallLog "${_HEADER}"
  ${WriteLogSeparator}
!macroend
!define LogHeader "!insertmacro LogHeader"

/**
 * Adds a section message to the human readable log.
 *
 * @param   _MSG
 *          The message text to write to the log.
 */
!macro LogMsg _MSG
  FileWrite $fhInstallLog "  ${_MSG}$\r$\n"
!macroend
!define LogMsg "!insertmacro LogMsg"

/**
 * Adds an uninstall entry to the uninstall log.
 *
 * @param   _MSG
 *          The message text to write to the log.
 */
!macro LogUninstall _MSG
  FileWrite $fhUninstallLog "${_MSG}$\r$\n"
!macroend
!define LogUninstall "!insertmacro LogUninstall"

/**
 * Adds a section divider to the human readable log.
 */
!macro WriteLogSeparator
  FileWrite $fhInstallLog "$\r$\n----------------------------------------\
                           ---------------------------------------$\r$\n"
!macroend
!define WriteLogSeparator "!insertmacro WriteLogSeparator"
