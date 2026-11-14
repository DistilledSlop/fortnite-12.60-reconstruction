using UnrealBuildTool;

public class FortniteLinuxTarget : TargetRules
{
    public FortniteLinuxTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.Latest;
        // The original reload scope consumes the real engine hitch detector.
        GlobalDefinitions.Add("ALLOW_HITCH_DETECTION=1");
        ExtraModuleNames.Add("FortniteGame");
        ExtraModuleNames.Add("HoagieRuntime");
        ExtraModuleNames.Add("FortniteUI");
        ExtraModuleNames.Add("SocialUMG");
        ExtraModuleNames.Add("KairosSceneCapture");
        ExtraModuleNames.Add("EpicCMSUIFramework");
        ExtraModuleNames.Add("SubtitlesWidgets");
    }
}
