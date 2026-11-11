// CONFIDENCE: HIGH - inverted flag store and indicator sync are exact (0x1426b3990); the indicator IsHidden() mapping is MEDIUM.
// Original addresses: exec 0x142b48a00 -> native 0x1426b3990.
bTrajectoryHidden = !bNewVisible;
if (AttachedTrajectoryIndicator.IsValid())
{
    AActor* Indicator = AttachedTrajectoryIndicator.Get();
    if (Indicator->IsHidden() == bNewVisible)
    {
        Indicator->SetActorHiddenInGame(!bNewVisible);
    }
}
