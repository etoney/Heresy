; Heresy Windows installer (Inno Setup 6). Built by GitHub Actions:
;   iscc /DAppVersion=1.0.0 /DArtefacts=<build>\Heresy_artefacts\Release /DOutDir=<dist> heresy.iss

#ifndef AppVersion
  #define AppVersion "1.0.0"
#endif
#ifndef Artefacts
  #define Artefacts "..\..\build\Heresy_artefacts\Release"
#endif
#ifndef OutDir
  #define OutDir "..\..\dist"
#endif

[Setup]
AppId={{3E9B99D1-E393-47F4-999B-96338585AC7F}
AppName=Heresy
AppVersion={#AppVersion}
AppVerName=Heresy {#AppVersion}
AppPublisher=Castle Danger Music Company
AppPublisherURL=https://castledangermusic.com
DefaultDirName={autopf}\Castle Danger Music Company\Heresy
DefaultGroupName=Castle Danger Music Company
DisableProgramGroupPage=yes
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
OutputDir={#OutDir}
OutputBaseFilename=Heresy-{#AppVersion}-windows-setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
LicenseFile=license.txt
UninstallDisplayName=Heresy (Castle Danger Music Company)

[Types]
Name: "full"; Description: "VST3 plugin and standalone app"
Name: "custom"; Description: "Choose"; Flags: iscustom

[Components]
Name: "vst3"; Description: "VST3 plugin (C:\Program Files\Common Files\VST3)"; Types: full custom
Name: "app"; Description: "Standalone app"; Types: full

[Files]
Source: "{#Artefacts}\VST3\Heresy.vst3\*"; DestDir: "{commoncf64}\VST3\Heresy.vst3"; Components: vst3; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#Artefacts}\Standalone\Heresy.exe"; DestDir: "{app}"; Components: app; Flags: ignoreversion

[Icons]
Name: "{group}\Heresy"; Filename: "{app}\Heresy.exe"; Components: app
Name: "{group}\Uninstall Heresy"; Filename: "{uninstallexe}"

[Run]
Filename: "{app}\Heresy.exe"; Description: "Open Heresy now"; Components: app; Flags: nowait postinstall skipifsilent unchecked

[Messages]
WelcomeLabel2=This installs Heresy, the horror sound design synth from Castle Danger Music Company.%n%nYour own presets live in %APPDATA%\Castle Danger Music Company\Heresy\Presets and are never touched by updates.
FinishedLabel=Heresy is installed. Restart your DAW or rescan plugins, then load Heresy from the instruments list.
