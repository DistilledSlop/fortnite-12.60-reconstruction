// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b3d0d0; see evidence.json for callees.
// Signature: void ResaveActor(AActor* Actor)
if (Role != ROLE_Authority)
{
    return;
}
ResaveActorWithVolume(Actor, GetVolumeForActor(Actor));
