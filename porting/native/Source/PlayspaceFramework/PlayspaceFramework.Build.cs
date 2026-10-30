using System.IO;
using UnrealBuildTool;

// Runtime module that owns the Playspace classes (APlayspaceGameState, APlayspacePlayerController).
// The reflection tables place them in /Script/PlayspaceFramework, and that package name is part of the
// class path on the wire, so the module must carry this exact name. See porting/native_classes_status.md.
public class PlayspaceFramework : ModuleRules
{
    public PlayspaceFramework(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine" });
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Generated"));
    }
}
