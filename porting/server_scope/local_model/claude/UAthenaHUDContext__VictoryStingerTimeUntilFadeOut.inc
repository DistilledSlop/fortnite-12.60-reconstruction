// CONFIDENCE: MEDIUM - same chain as GetVictoryStinger; the fallback value -1.0f (0xbf800000) is read directly from the native.
// Original addresses: exec 0x1451a0930; body 0x144cb4d20
UWorld* World = GetWorld();
AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GameState) : nullptr;
UFortPlaylistUIInfo* UIInfo = GameState ? NativeGetPlaylistUIData(GameState) : nullptr;
return UIInfo ? UIInfo->StingerFadesToAudioMusicAfter : -1.0f;
