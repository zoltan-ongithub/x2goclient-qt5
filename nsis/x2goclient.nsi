;NSIS Modern User Interface
;Multilingual Example Script
;Written by Joost Verburg


!include "MUI.nsh"
!include "FileFunc.nsh"
!include "Sections.nsh"
!insertmacro Locate
;--------------------------------
;General

  RequestExecutionLevel admin
 !define VERSION "X2GOCLIENT_VERSION"
  Name "x2goclient ${VERSION}"
  Caption "x2goclient ${VERSION}"
  OutFile "x2goclient-${VERSION}-setup.exe"
  !define MUI_ICON icons\win-install.ico
  !define MUI_UNICON icons\win-uninstall.ico
  ;Default installation folder
  InstallDir "$PROGRAMFILES\x2goclient"

  ;NSIS 2.46 defaults to zlib. Setting this reduces the size of a default
  ;(no fonts) build by about 24%
  SetCompressor /SOLID lzma

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "Software\x2goclient" ""

;--------------------------------
;Check if user is admin
Section
    userInfo::getAccountType
    pop $0
    strCmp $0 "Admin" +3
    messageBox MB_OK $(ADM_RIGHT)
    abort
SectionEnd

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKLM"
  !define MUI_LANGDLL_REGISTRY_KEY "Software\x2goclient"
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"

;------------------------------
;Variables

  Var STARTMENU_FOLDER
  Var MUI_TEMP

  !define  UNINSTALL_REGKEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\x2goclient"
  !define  UNINSTALL_DISPLAYNAME "X2Go Client for Windows"
  !define  UNINSTALL_PUBLISHER "X2Go Project"
  !define  UNINSTALL_DISPLAYVERSION ${VERSION}
  !define  UNINSTALL_URL "http://www.x2go.org"

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "gpl.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  !define MUI_STARTMENUPAGE_DEFAULTFOLDER "X2Go Client for Windows"
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\x2goclient"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU "Application" $STARTMENU_FOLDER
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "Russian"

;--------------------------------
;Reserve Files

  ;If you are using solid compression, files that are required before
  ;the actual installation should be stored first in the data block,
  ;because this will make your installer start faster.

  !insertmacro MUI_RESERVEFILE_LANGDLL


;--------------------------------
;Installer Sections

;"Recommended" is the default because it is specified 1st.
InstType "Recommended"
InstType "Full"
InstType "Minimal"

Section "X2Go Client (required)" base

  SetShellVarContext all
  SectionIn RO

  SetOutPath "$INSTDIR"
  File /a /x x2goclient.debug.exe "x2goclient\*.*"
  File /r "x2goclient\pulse"
  File /r /x "fonts" "x2goclient\VcXsrv"

  ;Store installation folder
  WriteRegStr HKLM "Software\x2goclient" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  CreateShortCut "$INSTDIR\X2Go Client.lnk" "$INSTDIR\x2goclient.exe"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\X2Go Client.lnk" "$INSTDIR\x2goclient.exe"

  ;Cleanup previous icon names (prior to X2Go Client 4.0.1.2)
  Delete "$INSTDIR\X2goClient.lnk"
  Delete "$SMPROGRAMS\$STARTMENU_FOLDER\X2goClient.lnk"
  Delete "$DESKTOP\X2goClient.lnk"

  !insertmacro MUI_STARTMENU_WRITE_END

  ;Add uninstall information to Add/Remove Programs
  ;http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "InstallLocation"  "$INSTDIR"
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "UninstallString"  "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "DisplayIcon"      "$INSTDIR\x2goclient.exe"
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "DisplayName"      "${UNINSTALL_DISPLAYNAME}"
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "Publisher"        "${UNINSTALL_PUBLISHER}"
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "DisplayVersion"   "${UNINSTALL_DISPLAYVERSION}"
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "HelpLink"         "${UNINSTALL_URL}"
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "URLInfoAbout"     "${UNINSTALL_URL}"
  WriteRegStr HKLM ${UNINSTALL_REGKEY} "URLUpdateInfo"    "${UNINSTALL_URL}"
  WriteRegDWORD HKLM ${UNINSTALL_REGKEY} "NoModify"       1
  WriteRegDWORD HKLM ${UNINSTALL_REGKEY} "NoRepair"       1

SectionEnd

;x2goclient bug 108 fix
SectionGroup "Fonts" fonts

  ;Empirical testing shows that "misc" fixes compatibility for the majority
  ;of applications with font compatibility programs.
  ;So lets make "misc" be part of "recommended", and therefore the default.
  ;
  ;As of VcXsrv-xp 1.14.3.2
  ;misc is 412 files at 6.80 MB (7.94 MB on disk)
  Section "misc" fonts-misc
    SectionIn 1 2
    SetOutPath "$INSTDIR\VcXsrv\fonts\"
    File "x2goclient\VcXsrv\fonts\fonts.conf"
    File /r "x2goclient\VcXsrv\fonts\misc"
  SectionEnd

  ;As of VcXsrv-xp 1.14.3.2
  ;75dpi is 1,897 files at 10.7 MB (15.6 MB on disk)
  Section "75dpi" fonts-75dpi
    SectionIn 2
    SetOutPath "$INSTDIR\VcXsrv\fonts\"
    File "x2goclient\VcXsrv\fonts\fonts.conf"
    File /r "x2goclient\VcXsrv\fonts\75dpi"
  SectionEnd

  ;As of VcXsrv-xp 1.14.3.2
  ;100dpi is 1,897 files at 12.3 MB (16.8 MB on disk)
  Section "100dpi" fonts-100dpi
    SectionIn 2
    SetOutPath "$INSTDIR\VcXsrv\fonts\"
    File "x2goclient\VcXsrv\fonts\fonts.conf"
    File /r "x2goclient\VcXsrv\fonts\100dpi"
  SectionEnd

  ;As of VcXsrv-xp 1.14.3.2
  ;everything else is 466 files at 12.8MB (13.8 MB on disk)
  Section "others" fonts-others
    SectionIn 2
    SetOutPath "$INSTDIR\VcXsrv\fonts\"
    File "x2goclient\VcXsrv\fonts\fonts.conf"
    File /r "x2goclient\VcXsrv\fonts\cyrillic"
    File /r "x2goclient\VcXsrv\fonts\encodings"
    File /r "x2goclient\VcXsrv\fonts\OTF"
    File /r "x2goclient\VcXsrv\fonts\Speedo"
    File /r "x2goclient\VcXsrv\fonts\terminus-font"
    File /r "x2goclient\VcXsrv\fonts\TTF"
    File /r "x2goclient\VcXsrv\fonts\Type1"
  SectionEnd

SectionGroupEnd

Section "Desktop Shortcut" desktopshortcut
  SectionIn 1 2
  # When you run SetOutPath,you set the "Start in" dir for the shortcut.
  # This "Start in" dir must exist for the shortcut to work.
  SetOutPath "$INSTDIR"
  CreateShortCut "$DESKTOP\X2Go Client.lnk" "$INSTDIR\x2goclient.exe"
SectionEnd

Section "Debug Build" debugbuild
    SectionIn 2
    SetOutPath "$INSTDIR"
    File "x2goclient\x2goclient.debug.exe"
    CreateShortCut "$INSTDIR\X2Go Client (Debug).lnk" "$INSTDIR\x2goclient.debug.exe" "--debug"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\X2Go Client (Debug).lnk" "$INSTDIR\x2goclient.debug.exe" "--debug"
SectionEnd

Section -EstimatedSize
  SectionIn RO
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM ${UNINSTALL_REGKEY} "EstimatedSize" "$0"
SectionEnd

;-------------------------------------------
;Descriptions


LangString ADM_RIGHT ${LANG_ENGLISH} "You have to be Administrator on this computer to install X2Go Client"
LangString ADM_RIGHT ${LANG_GERMAN} "Sie brauchen Administratorenrechte um X2go Client zu installieren"
LangString ADM_RIGHT ${LANG_RUSSIAN} "Для того, чтобы установить X2Go Client, необходимо быть администратором этого компьютера"

LangString DESC_base            ${LANG_ENGLISH} "The regular build of X2Go Client and all its required dependencies"
LangString DESC_fonts           ${LANG_ENGLISH} "Fonts are required for certain legacy/proprietary apps to run properly."
LangString DESC_desktopshortcut ${LANG_ENGLISH} "Desktop shortcut"
LangString DESC_debugbuild      ${LANG_ENGLISH} "A build of X2Go Client with console debugging output. Install and use this if you are reporting a bug."

!Insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${base}            $(DESC_base)
  !insertmacro MUI_DESCRIPTION_TEXT ${fonts}           $(DESC_fonts)
  !insertmacro MUI_DESCRIPTION_TEXT ${fonts-misc}      $(DESC_fonts)
  !insertmacro MUI_DESCRIPTION_TEXT ${fonts-75dpi}     $(DESC_fonts)
  !insertmacro MUI_DESCRIPTION_TEXT ${fonts-100dpi}    $(DESC_fonts)
  !insertmacro MUI_DESCRIPTION_TEXT ${fonts-others}    $(DESC_fonts)
  !insertmacro MUI_DESCRIPTION_TEXT ${desktopshortcut} $(DESC_desktopshortcut)
  !insertmacro MUI_DESCRIPTION_TEXT ${debugbuild}      $(DESC_debugbuild)
!Insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Installer Functions

Function .onInit

  !insertmacro MUI_LANGDLL_DISPLAY
FunctionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  SetShellVarContext all

  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP
  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\X2Go Client.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\X2Go Client (Debug).lnk"
  Delete "$DESKTOP\X2Go Client.lnk"
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"
  startMenuDeleteLoop:
    ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."
    IfErrors startMenuDeleteLoopDone
    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  RMDir /r "$INSTDIR"

  DeleteRegKey HKLM "Software\x2goclient"
  DeleteRegKey HKLM "${UNINSTALL_REGKEY}"

SectionEnd

;--------------------------------
;Uninstaller Functions

Function un.onInit

  !insertmacro MUI_UNGETLANGUAGE

FunctionEnd
