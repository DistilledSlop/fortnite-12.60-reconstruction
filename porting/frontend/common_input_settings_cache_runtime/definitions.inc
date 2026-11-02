// The eventual CommonInput module owns the matching category definition.
DECLARE_LOG_CATEGORY_EXTERN(LogCommonInput, Log, All);

void UCommonInputSettings::ValidateNativeInputDataCache()
{
    // IsPending tests the underlying soft object, without the typed class check.
    NativeInputDataLoaded &= !InputData.IsPending();
    const TArray<TSoftClassPtr<UCommonInputControllerData>> Snapshot = CurrentPlatform.ControllerData;
    for (TSoftClassPtr<UCommonInputControllerData> ControllerData : Snapshot)
    {
        bool bFound = false;
        for (const TSubclassOf<UCommonInputControllerData>& Existing : CurrentPlatform.ControllerDataClasses)
        {
            // Resolve for each comparison, including null-to-null comparisons.
            UClass* ExistingClass = Existing.Get();
            UClass* SoftClass = ControllerData.Get();
            if (ExistingClass == SoftClass)
            {
                bFound = true;
                break;
            }
        }
        NativeInputDataLoaded &= bFound;
        NativeInputDataLoaded &= !ControllerData.IsPending();
    }

    if (!NativeInputDataLoaded)
    {
        UE_LOG(LogCommonInput, Warning,
            TEXT("Trying to access unloaded CommmonInputSettings data. This may force a sync load."));
    }
    InitializeNativeInputDataCache();
}

void UCommonInputSettings::InitializeNativeInputDataCache()
{
    if (NativeInputDataLoaded)
    {
        return;
    }

    const bool bRootResolvedClasses = GUObjectArray.IsDisregardForGC(this);
    InputDataClass = InputData.LoadSynchronous();
    if (InputDataClass && bRootResolvedClasses)
    {
        InputDataClass.Get()->AddToRoot();
    }

    CurrentPlatform = CommonInputPlatformData[0];
    const TArray<TSoftClassPtr<UCommonInputControllerData>> Snapshot = CurrentPlatform.ControllerData;
    for (TSoftClassPtr<UCommonInputControllerData> ControllerData : Snapshot)
    {
        UClass* ControllerClass = ControllerData.LoadSynchronous();
        if (ControllerClass)
        {
            CurrentPlatform.ControllerDataClasses.Add(ControllerClass);
            if (bRootResolvedClasses)
            {
                ControllerClass->AddToRoot();
            }
        }
    }
    NativeInputDataLoaded = true;
}

FCommonInputPlatformData UCommonInputSettings::NativeGetCurrentPlatformData()
{
    ValidateNativeInputDataCache();
    return CurrentPlatform;
}
