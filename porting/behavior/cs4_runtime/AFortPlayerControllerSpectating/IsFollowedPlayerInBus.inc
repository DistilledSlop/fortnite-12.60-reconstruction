// Behaviour snippet: AFortPlayerControllerSpectating::IsFollowedPlayerInBus (body only; spliced into the generated member function).
// Original body: 0x1423d9010 (thunk 0x142a6be70).
// Evidence: virtual slot +0x2058 = NativeGetSpectatingPlayerState163; IsA check against AFortPlayerStateAthena::StaticClass() (ClassDepth/ClassAncestors chain); test [PlayerState+0xd00] & 0x1 = AFortPlayerStateZone::bInAircraft (bool_bits.json).
// CONFIDENCE: HIGH
AFortPlayerState* SpectatedState = NativeGetSpectatingPlayerState163();
if (SpectatedState == nullptr)
{
    return false;
}
static_assert(sizeof(AFortPlayerStateAthena) > 0, "AFortPlayerStateAthena must be complete for the class check");
AFortPlayerStateAthena* SpectatedAthena = Cast<AFortPlayerStateAthena>(SpectatedState);
return SpectatedAthena != nullptr && SpectatedAthena->bInAircraft;
