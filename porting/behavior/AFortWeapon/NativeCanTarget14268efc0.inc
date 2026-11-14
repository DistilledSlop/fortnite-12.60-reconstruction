// CONFIDENCE: HIGH. Complete original body.
// Original14268efc0. Full123-byte body independently reviewed from raw image.
if (bAllowTargeting) return true;
if (UWorld* World = GetWorld())
{
    if (AFortGameStateAthena* State = Cast<AFortGameStateAthena>(World->GetGameState()))
    {
        return State->NativeQueryWeaponTargeting1419b08a0(this);
    }
}
return false;
