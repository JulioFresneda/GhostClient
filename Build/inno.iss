[Setup]
AppName=GhostStream
AppVersion=1.0
DefaultDirName={pf}\GhostStream
DefaultGroupName=GhostStream
OutputDir=Output
OutputBaseFilename=GhostSetup

WizardImageFile=logo.bmp
; Optional: Add a smaller image in the top-right corner
WizardSmallImageFile=logo.bmp

[Files]
; All files in the same directory as the script
Source: "*.*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs


[Icons]
Name: "{group}\GhostStream"; Filename: "{app}\GhostStream.exe"
Name: "{commondesktop}\GhostStream"; Filename: "{app}\GhostStream.exe"

Name: "{group}\Write your ghostly pass"; Filename: "{app}\conf.ini"; 
