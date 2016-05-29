; munin-node-win32.nsi
;
; NSIS Installer build script
;--------------------------------

!include "MUI2.nsh"

!addplugindir "nsisFirewall"
!include "FileFunc.nsh"

; The name of the installer
!define VERSION 1.6.1.0
Name "Munin Node for Windows ${VERSION} (Beta)"

!ifdef MUNIN_ARCH_X64
	!define ARCH x64
!else
	!define ARCH win32
!endif

; The file to write
OutFile "munin-node-${ARCH}-${VERSION}-installer.exe"

; The default installation directory
!ifdef MUNIN_ARCH_X64
InstallDir "$PROGRAMFILES64\Munin Node for Windows"
!else
InstallDir "$PROGRAMFILES\Munin Node for Windows"
!endif

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Munin Node for Windows" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

VIProductVersion "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "Munin Node for Windows"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "NSIS Installer for Munin Node for Windows"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2006-2011 Jory 'jcsston' Stone, modified by Adam Groszer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Munin Node for Windows"
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "munin-node-${ARCH}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"
  
;--------------------------------

!define MUI_ABORTWARNING

; Pages
!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "Munin Node for Windows (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
!ifdef MUNIN_ARCH_X64
  File "..\bin.x64\Release\munin-node.exe"
!else
  File "..\bin\Release\munin-node.exe"
!endif
  File "..\munin-node.ini"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Munin Node for Windows" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "DisplayName" "Munin Node for Windows"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "DisplayIcon " '"$INSTDIR\munin-node.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "URLInfoAbout" "http://code.google.com/p/munin-node-win32/"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "NoRepair" 1
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "EstimatedSize" "$0"
 
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  CreateDirectory "$SMPROGRAMS\Munin Node for Windows"
  CreateShortCut "$SMPROGRAMS\Munin Node for Windows\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  CreateShortCut "$SMPROGRAMS\Munin Node for Windows\Run munin-node in foreground.lnk" "$INSTDIR\munin-node.exe" "-run" "$INSTDIR\munin-node.exe" 0
  
SectionEnd

; Add firewall rule
Section "Add Windows Firewall Rule"
	; Add Munin Node for Windows to the authorized list
	nsisFirewall::AddAuthorizedApplication "$INSTDIR\munin-node.exe" "Munin Node for Windows"
	Pop $0
	IntCmp $0 0 +3
		MessageBox MB_OK "A problem happened while adding Munin Node for Windows to the Firewall exception list (result=$0)"
		Return
SectionEnd

; Install service
Section "Install Service"
	ExecWait '"$INSTDIR\munin-node.exe" -install'
SectionEnd

;--------------------------------

; Uninstaller

Section "Uninstall"
	; Remove Munin Node for Windows from the authorized list
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\munin-node.exe"
	Pop $0
	IntCmp $0 0 +3
		MessageBox MB_OK "A problem happened while removing Munin Node for Windows from the Firewall exception list (result=$0)"
		Return

  ; Remove service
  ExecWait '"$INSTDIR\munin-node.exe" -uninstall'
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows"
  DeleteRegKey HKLM "SOFTWARE\Munin Node for Windows"

  ; Remove files and uninstaller
  Delete $INSTDIR\munin-node.exe
  Delete $INSTDIR\uninstall.exe

  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Munin Node for Windows\*.*"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Munin Node for Windows"
  RMDir "$INSTDIR"

SectionEnd
