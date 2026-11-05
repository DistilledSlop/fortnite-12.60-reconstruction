// CONFIDENCE: high
// Native body 0x141ecc540. The original callback does not test bClearAllFog.
if ((NativeMapState_0x4c0 & uint32(0x20)) == 0)
{
    if (MapData.IsValidIndex(NativeCurrentTeamMapIndex_0x4b8))
    {
        FTeamMapInfo& TeamMap = MapData[NativeCurrentTeamMapIndex_0x4b8];
        FMemory::Memset(TeamMap.NativeFogPixels_0x20.GetData(), 0xff, TeamMap.NativeFogPixels_0x20.Num());
        FMemory::Memset(TeamMap.NativeRevealBits_0x10.GetData(), 0xff, TeamMap.NativeRevealBits_0x10.Num());
        if ((NativeMapState_0x4c0 & uint32(1)) != 0)
        {
            NativeUpdateMapTexture();
        }
        else
        {
            NativeMapState_0x4c0 |= uint32(2);
        }
    }
    UWorld* World = GetWorld();
    AFortGameStateZone* GameState = World ? Cast<AFortGameStateZone>(World->GetGameState()) : nullptr;
    if (GameState && GameState->VisibilityManager)
    {
        GameState->VisibilityManager->NativeRefreshAllVisibility();
    }
}
