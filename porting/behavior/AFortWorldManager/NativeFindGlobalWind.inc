// CONFIDENCE: HIGH. Original 0x141f87000. The game-state difficulty handle
// gates discovery only; the selection handle remains empty outside the PIE override.
const uint64 InvalidWindBits = 0x42340000b8d1b717ull;
FMemory::Memcpy(&OutWind, &InvalidWindBits, sizeof(InvalidWindBits));
UWorld* World = GetWorld();
AFortGameStateZone* GameState = Cast<AFortGameStateZone>(World->GetGameState());
if (!GameState)
    return false;
FDataTableRowHandle SelectionDifficulty;
if (GameState->ZoneDifficultyInfoRow.IsNull())
{
    if (GetWorld()->WorldType != EWorldType::PIE ||
        !NativeGetDifficultyRowForPIE(SelectionDifficulty))
        return false;
}
if (AFortGameMode* GameMode = Cast<AFortGameMode>(World->GetAuthGameMode()))
{
    const FGameplayTagContainer* ContextTags = GameMode->GetGameContextTagsNative();
    if (UFortTimeOfDayData* WindData = UFortTimeOfDayData::NativeGetGlobalWindData())
        if (WindData->NativeFindGlobalWindForContext(*ContextTags, OutWind))
            return true;
}

// 0x141f886f0: current record, then the first valid queued world-record action,
// then the process-wide empty zone instance. Only the first action is examined.
const FFortZoneInstanceDetails* Details = nullptr;
if (UFortWorldRecord* Record = CurrentWorldRecord)
    Details = &Record->ZoneInstanceInfo;
else if (NativeWorldRecordActions_0x6c0.Num() > 0)
{
    const FFortZoneInstanceDetails& First = NativeWorldRecordActions_0x6c0[0].ZoneInstanceInfo;
    if (!First.ZoneThemeClass.IsNull() && (!First.WorldId.IsEmpty() || !First.TheaterId.IsEmpty()))
        Details = &First;
}
if (!Details)
{
    static const FFortZoneInstanceDetails EmptyDetails = []()
    {
        FFortZoneInstanceDetails Value{};
        Value.TileIndex = INDEX_NONE;
        return Value;
    }();
    Details = &EmptyDetails;
}

// The class resolvers track even empty-path attempts. Their load fallback can
// return an unrelated UClass; only the tracked reference is subtype-filtered.
const auto ResolveClass = [](UClass* Cached, const FSoftObjectPath& Path, UClass* Expected) -> UClass*
{
    UClass* Result = Cached;
    if (!Result || !Result->IsChildOf(Expected))
        Result = Cast<UClass>(FortLoadDelayedObject(Path, true));
    FFortAssets::Get().TrackAsset(Result && Result->IsChildOf(Expected) ? Result : nullptr);
    return Result;
};
UClass* MissionClass = ResolveClass(Details->MissionGenerator.Get(),
    Details->MissionGenerator.ToSoftObjectPath(), UFortMissionGenerator::StaticClass());
if (MissionClass && MissionClass->IsChildOf(UFortMissionGenerator::StaticClass()))
{
    UFortMissionGenerator* Generator = MissionClass->GetDefaultObject<UFortMissionGenerator>();
    const auto& MissionReference = Generator->PrimaryMissionInfo;
    if (!MissionReference.IsNull())
    {
        UFortMissionInfo* Mission = MissionReference.Get();
        if (!Mission)
            Mission = Cast<UFortMissionInfo>(FortLoadDelayedObject(MissionReference.ToSoftObjectPath(), true));
        FFortAssets::Get().TrackAsset(Mission);
        if (Mission && Mission->NativeFindGlobalWindForDifficulty(SelectionDifficulty, OutWind))
            return true;
    }
}
UFortZoneTheme* ZoneTheme = nullptr;
if (Details->ZoneThemeClass.IsNull())
{
    if (World->WorldType != EWorldType::PIE)
        return false;
    if (ZoneThemeForPIE)
        ZoneTheme = ZoneThemeForPIE->GetDefaultObject<UFortZoneTheme>();
}
else
{
    UClass* ZoneClass = ResolveClass(Details->ZoneThemeClass.Get(),
        Details->ZoneThemeClass.ToSoftObjectPath(), UFortZoneTheme::StaticClass());
    if (ZoneClass && ZoneClass->IsChildOf(UFortZoneTheme::StaticClass()))
    {
        // Original 0x141f36b20 rereads the persistent soft pointer after resolving.
        UClass* ResolvedClass = Details->ZoneThemeClass.Get();
        ZoneTheme = ResolvedClass->GetDefaultObject<UFortZoneTheme>();
    }
}
return ZoneTheme && ZoneTheme->NativeFindGlobalWindForDifficulty(SelectionDifficulty, OutWind);
