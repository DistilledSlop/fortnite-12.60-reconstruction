using UnrealBuildTool;

public class FortniteLinuxServerTarget : TargetRules
{
    public FortniteLinuxServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("FortniteGame");
        ExtraModuleNames.Add("FortniteUI");
    }
}
