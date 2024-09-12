; Define the application name, version, and other details
[Setup]
AppName=cortexcpp-beta
AppVersion=1.0
DefaultDirName={localappdata}\cortexcpp-beta
DefaultGroupName=cortexcpp-beta
OutputDir=.
OutputBaseFilename=setup
Compression=lzma
SolidCompression=yes
PrivilegesRequired=lowest
AllowNoIcons=yes

; Define the languages section
[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

; Define the files to be installed
[Files]
Source: "cortex-beta.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "msvcp140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "vcruntime140_1.dll"; DestDir: "{app}"; Flags: ignoreversion

; Define the icons to be created
[Icons]
Name: "{group}\cortexcpp-beta"; Filename: "{app}\cortex-beta.exe"

; Define the run section to execute the application after installation
[Run]
Filename: "{app}\cortex-beta.exe"; Parameters: "engines cortex.llamacpp install"; WorkingDir: "{app}"; StatusMsg: "Initializing cortex configuration..."; Flags: nowait postinstall
[Code]
procedure AddToUserPath;
var
  ExpandedAppDir: String;
  CmdLine: String;
  ResultCode: Integer;
begin
  ExpandedAppDir := ExpandConstant('{app}');
  
  CmdLine := Format('setx PATH "%s;%%PATH%%"', [ExpandedAppDir]);
  
  if Exec('cmd.exe', '/C ' + CmdLine, '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    if ResultCode = 0 then
      MsgBox('Successfully added to user PATH.', mbInformation, MB_OK)
    else
      MsgBox('Failed to update user PATH. Error code: ' + IntToStr(ResultCode), mbError, MB_OK);
  end
  else
  begin
    MsgBox('Failed to execute setx command.', mbError, MB_OK);
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssPostInstall then
  begin
    AddToUserPath;
  end;
end;

[Tasks]
Name: "desktopicon"; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: unchecked
Name: "quicklaunchicon"; Description: "Create a &Quick Launch icon"; GroupDescription: "Additional icons:"; Flags: unchecked

; Define icons for the additional tasks
[Icons]
Name: "{commondesktop}\cortexcpp-beta"; Filename: "{app}\cortex-beta.exe"; Tasks: desktopicon
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\cortexcpp-beta"; Filename: "{app}\cortex-beta.exe"; Tasks: quicklaunchicon

; Define the uninstall run section to execute commands before uninstallation
[UninstallRun]
Filename: "{app}\cortex-beta.exe"; Parameters: "stop"; StatusMsg: "Stopping cortexcpp-beta service..."; Flags: runhidden

; Use Pascal scripting to ask user if they want to delete the cortexcpp-beta folder and .cortexrc-beta file
[Code]
procedure DeleteCurrentUserCortexFolderAndConfig;
var
  UserCortexFolder: String;
  UserCortexConfig: String;
  ShouldDelete: Integer;
begin
  UserCortexFolder := ExpandConstant('{%USERPROFILE}\cortexcpp-beta');
  UserCortexConfig := ExpandConstant('{%USERPROFILE}\.cortexrc-beta');
  
  if DirExists(UserCortexFolder) or FileExists(UserCortexConfig) then
  begin
    ShouldDelete := MsgBox('Do you want to delete the application data in cortexcpp-beta and the .cortexrc-beta config file (this will remove all user data)?', mbConfirmation, MB_YESNO);
    
    if ShouldDelete = idYes then
    begin
      if DirExists(UserCortexFolder) then
      begin
        DelTree(UserCortexFolder, True, True, True);
      end;

      if FileExists(UserCortexConfig) then
      begin
        DeleteFile(UserCortexConfig);
      end;
    end;
  end;
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usPostUninstall then
  begin
    DeleteCurrentUserCortexFolderAndConfig;
  end;
end;