// CONFIDENCE: HIGH
// 0x141fbd1f0. Native class storage is checked only at the observed sites.
if (GetLocalRole() != ROLE_Authority)
    return nullptr;
UWorld* World = GetWorld();
if (!World || !World->bIsWorldInitialized || !World->IsGameWorld() ||
    !bSpawnTimeOfDayManager || !IsInPersistentLevel(false) || World->WorldType == EWorldType::Inactive)
    return nullptr;

TActorIterator<AFortTimeOfDayManager> Existing(World, AFortTimeOfDayManager::StaticClass(),
    EActorIteratorFlags::OnlyActiveLevels | EActorIteratorFlags::SkipPendingKill);
if (Existing)
{
    if (!TimeOfDayManager)
        TimeOfDayManager = *Existing;
    return TimeOfDayManager;
}

// UE's public Get() validates the subclass. The original also reads the raw
// single-pointer storage before CDO checks, including wrong-base classes.
const auto RawClass = [](const TSubclassOf<AFortTimeOfDayManager>& Value) -> UClass*
{
    static_assert(sizeof(Value) == sizeof(UClass*), "Native subclass pointer storage");
    UClass* Result = nullptr;
    FMemory::Memcpy(&Result, &Value, sizeof(Result));
    return Result;
};
UClass* Candidate = nullptr;
AFortGameMode* Mode = Cast<AFortGameMode>(World->GetAuthGameMode());
if (Mode)
{
    Candidate = RawClass(Mode->GetTimeOfDayManagerClassNative());
    if (!Candidate || !Candidate->IsChildOf(AFortTimeOfDayManager::StaticClass()))
    {
        FGameplayTagContainer* ContextTags = Mode->GetGameContextTagsNative();
        if (UFortGameInstance::NativeIsGameMode141ed8de0(nullptr, 0))
        {
            UGameDataSTW* GameData = UGameDataSTW::NativeGetGameDataSTW();
            UFortTimeOfDayData* Data = GameData->ResolveTimeOfDayDataNative();
            if (Data)
            {
                TSoftClassPtr<AFortTimeOfDayManager> Selected;
                for (FWorldAmbientPropertiesMatchingGameContext& Entry : Data->WorldAmbientPropertiesList)
                {
                    if (Entry.QueryToMatch.Matches(*ContextTags))
                    {
                        Selected = UFortTimeOfDayData::NativeSelectTimeOfDayManager(Entry.TimeOfDayTheme);
                        break;
                    }
                }
                if (!Selected.IsNull())
                    Candidate = RawClass(FortResolveTrackedTimeOfDayClass(Selected, true));
            }
        }
    }
}
if (!Mode || !Candidate || !Candidate->GetDefaultObject())
{
    Candidate = RawClass(FortResolveTrackedTimeOfDayClass(MissionTimeOfDayManager, true));
    if (!Candidate || !Candidate->GetDefaultObject())
    {
        Candidate = RawClass(FortResolveTrackedTimeOfDayClass(ZoneThemeTimeOfDayManager, true));
        if (!Candidate || !Candidate->GetDefaultObject())
        {
            Candidate = RawClass(FortResolveTrackedTimeOfDayClass(WorldTimeOfDayManager, true));
            if (!Candidate || !Candidate->GetDefaultObject())
                Candidate = RawClass(FortResolveTrackedTimeOfDayClass(DefaultWorldTimeOfDayManager, true));
        }
    }
}
AFortTimeOfDayManager* Result = nullptr;
if (Candidate && Candidate->GetDefaultObject())
{
    FActorSpawnParameters Parameters;
    Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Parameters.ObjectFlags = RF_Transient;
    Parameters.Owner = this;
    UClass* SpawnClass = Candidate->IsChildOf(AFortTimeOfDayManager::StaticClass()) ? Candidate : nullptr;
    TimeOfDayManager = static_cast<AFortTimeOfDayManager*>(World->SpawnActor(
        SpawnClass, static_cast<const FVector*>(nullptr), static_cast<const FRotator*>(nullptr), Parameters));
    Result = TimeOfDayManager;
    if (Result)
    {
        // 0x1406801c0 is an empty shipping body (bShowTimeOfDayManager argument).
        Result = TimeOfDayManager;
    }
}
return Result;
