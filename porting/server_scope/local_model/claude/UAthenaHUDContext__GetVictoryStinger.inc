// CONFIDENCE: MEDIUM - GameState, playlist and UIInfo chain read in full; the playlist member names at +0x1df8/+0x1df0 are proposed.
// Original addresses: exec 0x14519de60; body 0x144c8fcf0
UWorld* World = GetWorld();
AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GameState) : nullptr;
UFortPlaylistUIInfo* UIInfo = GameState ? NativeGetPlaylistUIData(GameState) : nullptr;
return UIInfo ? UIInfo->VictoryStinger : nullptr;
