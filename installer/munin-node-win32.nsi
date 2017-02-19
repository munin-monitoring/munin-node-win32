; munin-node-win32.nsi
;
; NSIS Installer build script
;--------------------------------

!include "MUI2.nsh"

!addplugindir "SimpleFC"
!addplugindir "SimpleSC"
!include "FileFunc.nsh"

; The name of the installer
!define VERSION 1.7.4.0
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

; always show details during installation
ShowInstDetails show

;--------------------------------

!define MUI_ABORTWARNING

; Icon for installer
!define MUI_ICON "..\munin.ico"

; Pages
!define MUI_WELCOMEPAGE_TITLE_3LINES
!insertmacro MUI_PAGE_WELCOME
!define MUI_COMPONENTSPAGE_NODESC
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_TITLE_3LINES
!define MUI_FINISHPAGE_NOAUTOCLOSE
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

;--------------------------------

VIProductVersion "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "Munin Node for Windows"
VIAddVersionKey /LANG=${LANG_ENGLISH} "Comments" "NSIS Installer for Munin Node for Windows"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "Copyright (C) 2006-2011 Jory 'jcsston' Stone, modified by Adam Groszer"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileDescription" "Munin Node for Windows"
VIAddVersionKey /LANG=${LANG_ENGLISH} "InternalName" "munin-node-${ARCH}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "${VERSION}"
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductVersion" "${VERSION}"

; Check if service already exists and uninstall if necessary
Section "Uninstall existing Service"

	SimpleSC::ExistsService "munin-node"
	Pop $0
	IntCmp $0 0 service_exists service_does_not_exist service_does_not_exist
	    service_exists:
		; Uninstall old service
		ExecWait '"$INSTDIR\munin-node.exe" -uninstall'
	service_does_not_exist:

SectionEnd

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
  SetOverwrite off
  File "..\munin-node.ini"
  SetOverwrite on
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Munin Node for Windows" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "DisplayName" "Munin Node for Windows"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "Publisher" "Munin Project"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "DisplayIcon" '"$INSTDIR\munin-node.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Munin Node for Windows" "URLInfoAbout" "https://github.com/munin-monitoring/munin-node-win32"
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
  CreateShortCut "$SMPROGRAMS\Munin Node for Windows\Run munin-node.lnk" "$INSTDIR\munin-node.exe" "-run" "$INSTDIR\munin-node.exe" 0
  CreateShortCut "$SMPROGRAMS\Munin Node for Windows\Install Service munin-node.lnk" "$INSTDIR\munin-node.exe" "-install" "$INSTDIR\munin-node.exe" 0
  CreateShortCut "$SMPROGRAMS\Munin Node for Windows\Uninstall Service munin-node.lnk" "$INSTDIR\munin-node.exe" "-uninstall" "$INSTDIR\munin-node.exe" 0
  
SectionEnd

; Add firewall rule
Section "Add Windows Firewall Rule"
	; Add Munin Node for Windows to the authorized list
	SimpleFC::AdvAddRule "Munin Node for Windows" "Allow incoming connections on TCP/4949 to munin-node.exe for all networks"  6 1 1 2147483647 1 "$INSTDIR\munin-node.exe" "" "" "Munin" 4949 "" "" ""
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
	SimpleFC::AdvRemoveRule "Munin Node for Windows"
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
