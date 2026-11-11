// CONFIDENCE: MEDIUM Original addresses: thunk 0x142b3d150; see evidence.json for callees.
// Signature: void ResaveActorWithVolume(AActor* Actor, AFortVolume* Volume)
// Shared native 0x141c8cfd0 is also described by deps/AFortVolumeManager__RegisterTrapInVolume.json; helper proposed as NativeResaveActorWithVolume.
if (Role != ROLE_Authority || !Volume)
{
    return;
}
NativeResaveActorWithVolume(Actor, Volume);
