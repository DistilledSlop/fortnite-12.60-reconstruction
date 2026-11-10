// CONFIDENCE: MEDIUM. Impl at 0x1423abb20 is fully read.
// The loading-screen flag is at this+0x719 (proposed name bLoadingScreenDropped).
AActor* World = GetWorld();   // virtual slot 0x150
if (World != nullptr)
{
    AFortGameMode* GameMode = World->AuthorityGameMode;   // UWorld+0x128
    if (GameMode && GameMode->IsA(AFortGameMode::StaticClass()))
    {
        GameMode->OnPlayerLoadingScreenDropped(this);   // virtual slot 0xb20 (target on the game mode not read; name proposed)
    }
}
bLoadingScreenDropped = true;   // set on every path
