// CONFIDENCE: LOW. The predicate FUN_14236df90 (CanKickPlayer) was read only to about 60 percent; FUN_140d7c5a0 is uncaptured.
// Original implementation address: 0x1423aba80 (virtual slot 0xf28 on AFortPlayerController).
if (MatchPhase != 3) { return; }   // byte at this+0xf0; 3 is the value the original gates on
if (!CanKickPlayer(PlayerToKick)) { return; }   // FUN_14236df90(this, PlayerToKick)
AFortGameMode* GameMode = GetWorld()->AuthorityGameMode;   // GetWorld = virtual slot 0x150; AuthorityGameMode at UWorld+0x128
if (GameMode && GameMode->IsA(AFortGameMode::StaticClass()))
{
    GameMode->KickPlayer(this, Reason, PlayerToKick);   // virtual slot 0xa00 on the game mode (target not read)
}
