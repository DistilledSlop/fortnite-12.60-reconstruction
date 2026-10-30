using UnrealBuildTool;

public class FortnitePortEditor : ModuleRules
{
    public FortnitePortEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PrivateDependencyModuleNames.AddRange(new[] { "Core", "CoreUObject", "Engine", "UnrealEd", "AssetRegistry", "FortniteGame" });
    }
}
