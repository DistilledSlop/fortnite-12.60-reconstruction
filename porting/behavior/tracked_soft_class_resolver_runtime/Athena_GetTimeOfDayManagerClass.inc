// CONFIDENCE: HIGH
// 0x141949030, virtual +0x930. Name proposed pending base virtual recovery.
if (TimeOfDayManagerGameplayOverride.Get())
    return TimeOfDayManagerGameplayOverride;
if (AFortGameStateAthena* AthenaState = Cast<AFortGameStateAthena>(GameState))
{
    if (UFortPlaylistAthena* Playlist = AthenaState->GetCurrentPlaylistAthena())
        return FortResolveTrackedTimeOfDayClass(Playlist->TimeOfDayManager, true);
}
return nullptr;
