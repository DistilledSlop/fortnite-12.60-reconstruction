// CONFIDENCE: HIGH
if (!bSynchronous)
{
    if (!(FPlatformTime::Seconds() - State.LastLoadTime >= 0.001))
    {
        return;
    }
    if (Path.Len() == State.LastLoadPath.Len() &&
        FGenericPlatformStricmp::Stricmp(*Path, *State.LastLoadPath) == 0)
    {
        return;
    }
    if (Path.StartsWith(TEXT("/Temp/Game/"), ESearchCase::IgnoreCase))
    {
        return;
    }
}
if (!NativeShouldSuppressLoadDiagnostic())
{
    const bool bAthena = NativeGetGameSubMode() == 1;
    int32& Count = bAthena ? State.AthenaLoadIssueCount : State.CampaignLoadIssueCount;
    Count = static_cast<int32>(static_cast<uint32>(Count) + 1u);
    // The original image is shipping (140e51650 returns 4), independent of
    // the configuration used to compile this reconstruction.
    const int32 Limit = NativeGetLoadIssueAnalyticsLimit(bAthena);
    if (Count <= Limit && GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            UWorld* World = Context.World();
            if (World && World->WorldType == EWorldType::Game)
            {
                APlayerController* FirstController = GEngine->GetFirstLocalPlayerController(World);
                if (Cast<AFortPlayerController>(FirstController))
                {
                    // Original calls the engine getter again after its type check.
                    AFortPlayerController* Controller = static_cast<AFortPlayerController*>(GEngine->GetFirstLocalPlayerController(World));
                    if (bAthena)
                    {
                        if (bSynchronous)
                            AFortPlayerController::NativeRecordAthenaSynchronousLoad(Controller, Path);
                        else
                            AFortPlayerController::NativeRecordAthenaAsynchronousLoad(Controller, Path);
                    }
                    else
                    {
                        if (bSynchronous)
                            AFortPlayerController::NativeRecordCoreSynchronousLoad(Controller, Path);
                        else
                            AFortPlayerController::NativeRecordCoreAsynchronousLoad(Controller, Path);
                    }
                    break;
                }
            }
        }
    }
    if (bSynchronous)
    {
        CSV_EVENT(Fort, TEXT("Sync load : %s"), *Path);
        UE_LOG(LogFort, Warning, TEXT("[LoadIssue] Synchronously loading file on client while match is in progress and loading screen is not shown:  %s"), *Path);
    }
}
State.LastLoadTime = FPlatformTime::Seconds();
State.LastLoadPath = Path;
