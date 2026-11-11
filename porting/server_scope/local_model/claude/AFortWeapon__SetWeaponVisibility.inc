// CONFIDENCE: MEDIUM - hide/notify, owner view-actor and trajectory steps are exact in structure; the bool order follows the FFrame read order (bDesiredVisibility first, bForceUpdate second), which conflicts with the reflection list.
// Original addresses: exec 0x142b48c60 -> native 0x1426b3b20 (+ 0x142b44250, 0x1419f38a0, 0x1426b3990).
const bool bCurrentlyVisible = !IsHidden();
if (bForceUpdate || bCurrentlyVisible != bDesiredVisibility)
{
    SetActorHiddenInGame(!bDesiredVisibility);
    NotifyVisibilityChanged(bDesiredVisibility, false);
}
if (AFortPlayerController* OwnerController = Cast<AFortPlayerController>(GetOwner()))
{
    if (AActor* ViewActor = OwnerController->GetRelevantWeaponViewActor(false))
    {
        SetActorVisibilityInternal(ViewActor, bDesiredVisibility);
    }
}
SetTrajectoryVisibility(bDesiredVisibility);
