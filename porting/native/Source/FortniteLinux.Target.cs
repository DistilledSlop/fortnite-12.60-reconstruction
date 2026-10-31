using UnrealBuildTool;

public class FortniteLinuxTarget : TargetRules
{
    public FortniteLinuxTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        ExtraModuleNames.Add("FortniteGame");
        ExtraModuleNames.Add("FortniteUI");
        ExtraModuleNames.Add("SocialUMG");
        ExtraModuleNames.Add("KairosSceneCapture");
    }
}
