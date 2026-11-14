// CONFIDENCE: HIGH
// Original 0x141be3020. Preserve bounds validity before world/navigation lookup.
if (NativeGetFoundationBounds().IsValid)
{
    UWorld* World = GetWorld();
    if (World)
    {
        UAthenaNavSystem* NavigationSystem = Cast<UAthenaNavSystem>(World->GetNavigationSystem());
        if (NavigationSystem)
        {
            NavigationSystem->NativeMarkBuildingFoundationDirty(this);
        }
    }
}
