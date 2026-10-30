using UnrealBuildTool;

public class FortniteLinuxEditorTarget : TargetRules
{
    public FortniteLinuxEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("FortniteGame");
        ExtraModuleNames.Add("FortnitePortEditor");
    }
}
