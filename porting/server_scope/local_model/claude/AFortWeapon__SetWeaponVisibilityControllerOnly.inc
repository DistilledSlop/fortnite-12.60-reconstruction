// CONFIDENCE: LOW - per-viewer hidden lists (0x14238e8a0 / 0x1423ae360) and the view-actor step are decompiled but not fully named; the flow is read in full. Bool order conflicts with the reflection list (see SetWeaponVisibility).
// Original addresses: exec 0x142b48d30 -> native 0x1426b3bd0.
AFortPlayerController* OwnerController = Cast<AFortPlayerController>(GetOwner());
UWorld* World = GetWorld();
if (!OwnerController || !World)
{
    return;
}
for (AFortWeapon* Weapon : GetAllWeaponActors(World))
{
    if (!Weapon->HasWeaponStats() || Weapon->GetOwningController() != OwnerController)
    {
        continue;
    }
    const bool bInHiddenList = Weapon->IsWeaponHiddenForViewer(this);
    if (bForceUpdate || (!bInHiddenList) != bDesiredVisibility)
    {
        Weapon->SetWeaponHiddenForViewer(this, !bDesiredVisibility);
        NotifyVisibilityChanged(bDesiredVisibility, true);
    }
    if (AActor* ViewActor = OwnerController->GetRelevantWeaponViewActor(false))
    {
        Weapon->SetWeaponHiddenForViewer(ViewActor, !bDesiredVisibility);
    }
    if (AttachedTrajectoryIndicator.IsValid())
    {
        AActor* Indicator = AttachedTrajectoryIndicator.Get();
        if ((!Weapon->IsWeaponHiddenForViewer(Indicator)) != bDesiredVisibility)
        {
            Weapon->SetWeaponHiddenForViewer(Indicator, !bDesiredVisibility);
        }
    }
}
