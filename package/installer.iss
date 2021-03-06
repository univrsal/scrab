; Taken from https://raw.githubusercontent.com/Palakis/obs-websocket/4.x-current/installer/installer.iss

#define MyAppName "@PROJECT"
#define MyAppVersion "@VERSION"
#define MyAppPublisher "univrsal"
#define MyAppURL "http://github.com/unvirsal/@PROJECT"

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{@APPID}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={code:GetDirName}
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputBaseFilename=@PROJECT-installer.@ARCH.@VERSION
Compression=lzma
SolidCompression=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Files]
Source: "@BUILDDIR\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\LICENSE"; Flags: dontcopy
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{group}\{cm:ProgramOnTheWeb,{#MyAppName}}"; Filename: "{#MyAppURL}"
Name: "{group}\{cm:UninstallProgram,{#MyAppName}}"; Filename: "{#MyAppName}"

[Code]
procedure InitializeWizard();
var
  GPLText: AnsiString;
  Page: TOutputMsgMemoWizardPage;
begin
  ExtractTemporaryFile('LICENSE');
  LoadStringFromFile(ExpandConstant('{tmp}\LICENSE'), GPLText);

  Page := CreateOutputMsgMemoPage(wpWelcome,
    'License Information', 'Please review the license terms before installing obs-websocket',
    'Press Page Down to see the rest of the agreement. Once you are aware of your rights, click Next to continue.',
    String(GPLText)
  );
end;

// credit where it's due :
// following function come from https://github.com/Xaymar/obs-studio_amf-encoder-plugin/blob/master/%23Resources/Installer.in.iss#L45
function GetDirName(Value: string): string;
var
	InstallPath: string;
begin
	// initialize default path, which will be returned when the following registry
	// key queries fail due to missing keys or for some different reason
	Result := '{pf}\obs-studio';
	// query the first registry value; if this succeeds, return the obtained value
	if RegQueryStringValue(HKLM32, 'SOFTWARE\OBS Studio', '', InstallPath) then
		Result := InstallPath
end;

