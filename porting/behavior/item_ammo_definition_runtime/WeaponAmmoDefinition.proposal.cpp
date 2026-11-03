// Proposal only. Requires the shared FFortAssets tracking service described in HANDOFF.md.
// Includes: Engine/AssetManager.h, Engine/StreamableManager.h, Misc/CommandLine.h,
// Misc/Parse.h, Stats/StatsMisc.h.
// The declarations are to be merged into their respective generated classes:
// UFortWorldItemDefinition:
//   virtual UFortWorldItemDefinition* GetAmmoWorldItemDefinitionNative() const;
//   UFUNCTION(BlueprintPure) UFortWorldItemDefinition* GetAmmoWorldItemDefinition_BP() const;
// UFortWeaponItemDefinition:
//   virtual UFortWorldItemDefinition* GetAmmoWorldItemDefinitionNative() const override;

// This must call the *shared* FFortAssets singleton's 0x141de9e00 equivalent,
// including null, current tracking scope, synchronization, and GC retention.
void NativeTrackFortAsset(UObject* Object);

static UObject* NativeLoadDelayedObject(const FSoftObjectPath& Path, bool bLogLoad)
{
    if (Path.IsNull())
        return nullptr;
    static const bool bShowDelayLoadTimes = FParse::Param(FCommandLine::Get(), TEXT("ShowDelayLoadTimes"));
    TUniquePtr<FScopeLogTime> Timer;
    if (bLogLoad && bShowDelayLoadTimes)
    {
        const FString Label = FString::Printf(TEXT("Loaded delay-load asset %s"), *Path.ToString());
        Timer = MakeUnique<FScopeLogTime>(*Label, nullptr, FConditionalScopeLogTime::ScopeLog_Seconds);
    }
    return UAssetManager::IsValid()
        ? UAssetManager::GetStreamableManager().LoadSynchronous(Path, false, nullptr)
        : Path.TryLoad();
}

UFortWorldItemDefinition* UFortWorldItemDefinition::GetAmmoWorldItemDefinitionNative() const
{
    return nullptr;
}
UFortWorldItemDefinition* UFortWorldItemDefinition::GetAmmoWorldItemDefinition_BP() const
{
    return GetAmmoWorldItemDefinitionNative();
}
UFortWorldItemDefinition* UFortWeaponItemDefinition::GetAmmoWorldItemDefinitionNative() const
{
    if (AmmoData.IsNull())
        return nullptr;
    UFortWorldItemDefinition* Result = AmmoData.Get();
    if (!Result)
        Result = Cast<UFortWorldItemDefinition>(NativeLoadDelayedObject(AmmoData.ToSoftObjectPath(), true));
    NativeTrackFortAsset(Result);
    return Result;
}
