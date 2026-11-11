// CONFIDENCE: LOW. Static lookup of the time-of-day manager for a world context object. Requires the world's game state to be a FortGameStateBase and a volume manager to resolve for the context. The volume comes from an actor context (FindVolumeForActor) or from a player-state context. Returns null at the first failed step.
// Original addresses: exec 0x142b0f290 (thunk; writes the result through param_3). Body 0x141f611f0. Helpers: 0x141ef3ab0 (world game state from context), 0x141c74820 (GetVolumeManagerForActor), 0x141c7c5c0 (FindVolumeForActor), 0x141c7d420 (player-state volume, LOW), 0x141f9f270 (time-of-day manager for volume, LOW), 0x1429234b0 (FortGameStateBase class), 0x1446acb00 (AActor class), 0x142a85850 (AFortPlayerState class).

AGameStateBase* GameStateBase = NativeGetGameStateFromContext(WorldContextObject);
if (GameStateBase == nullptr || !GameStateBase->IsA(AFortGameStateBase::StaticClass()))
{
    return nullptr;
}

AFortVolumeManager* VolumeManager = GetVolumeManagerForActor(WorldContextObject);
if (VolumeManager == nullptr)
{
    return nullptr;
}

AFortVolume* Volume = nullptr;
if (WorldContextObject != nullptr)
{
    if (WorldContextObject->IsA(AActor::StaticClass()))
    {
        Volume = FindVolumeForActor(VolumeManager, Cast<AActor>(WorldContextObject));
    }
    else if (WorldContextObject->IsA(AFortPlayerState::StaticClass()))
    {
        Volume = NativeFindVolumeForPlayerState(VolumeManager, WorldContextObject);
    }
}

return NativeGetTimeOfDayManagerForVolume(Volume);
