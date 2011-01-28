; munin-node-win32.nsi
;
; NSIS Installer build script

;--------------------------------

!addplugindir "nsisFirewall"

; The name of the installer
Name "Munin Node for Windows"

; The file to write
OutFile "munin-node-win32-installer.exe"

; The default installation directory
InstallDir "$PROGRAMFILES\Munin Node for Windows"

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\Munin Node for Windows" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------

; Pages

Page components
Page directory
Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

;--------------------------------

; The stuff to install
Section "Munin Node for Windows (required)"

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put file there
  File "..\Release\munin-node.exe"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM "SOFTWARE\Munin Node for Windows" "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "DisplayName" "Munin Node for Windows"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Example2" "NoRepair" 1
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

;--------------------------------

; Uninstaller

Section "Uninstall"
	; Remove Munin Node for Windows from the authorized list
	nsisFirewall::RemoveAuthorizedApplication "$INSTDIR\munin-node.exe"
	Pop $0
	IntCmp $0 0 +3
		MessageBox MB_OK "A problem happened while removing Munin Node for Windows from the Firewall exception list (result=$0)"
		Return
  
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
