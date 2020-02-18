; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

;Munin Node For Windows

#define MyAppName "Munin Node"
#define MyAppVersion "1.7.3.1"
#define MyAppExeName "munin-node.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{75326742-F9D5-494B-AFC1-E5CA4D8E5184}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputBaseFilename=Munin-Node-1.7.3.1-x86
OutputDir=InstallerBuild\
Compression=lzma
SolidCompression=yes
WizardStyle=modern

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "..\bin\Release\munin-node.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\munin-node.ini"; DestDir: "{app}"; Flags: ignoreversion
Source: "Install-Service.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "Uninstall-Service.bat"; DestDir: "{app}"; Flags: ignoreversion
Source: "README.txt"; DestDir: "{app}"; Flags: ignoreversion isreadme
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Run]
Filename: "{app}\Install-Service.bat"; Description: "Install as a Windows Service"; Flags: nowait runhidden
Filename: "{sys}\netsh.exe"; Parameters: "firewall add allowedprogram ""{app}\munin-node.exe"" ""Munin Node for Windows"" ENABLE ALL"; StatusMsg: "Windows Firewall exception"; Flags: runhidden; MinVersion: 0,5.01.2600sp2;

[UninstallRun]
Filename: {sys}\sc.exe; Parameters: "stop munin-node" ; Flags: runhidden
Filename: {sys}\sc.exe; Parameters: "delete munin-node" ; Flags: runhidden
Filename: {sys}\netsh.exe; Parameters: "firewall delete allowedprogram program=""{app}\munin-node.exe"""; Flags: runhidden; MinVersion: 0,5.01.2600sp2;

