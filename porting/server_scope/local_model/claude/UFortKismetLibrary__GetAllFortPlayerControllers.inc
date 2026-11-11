// CONFIDENCE: MEDIUM
// Native 0x14296bdb0 -> 0x141eefea0: walks the world's player controllers and keeps AFortPlayerController instances.
// Spectating controllers are kept when bIncludeSpectators is set; all other Fort controllers when bIncludeNonSpectators is set.
TArray<AFortPlayerController*> Controllers;
const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
if (!World)
{
    return Controllers;
}
for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
{
    AFortPlayerController* FortController = Cast<AFortPlayerController>(It->Get());
    if (!FortController)
    {
        continue;
    }
    const bool bIsSpectator = FortController->IsA(AFortPlayerControllerSpectating::StaticClass());
    if (bIsSpectator ? bIncludeSpectators : bIncludeNonSpectators)
    {
        Controllers.Add(FortController);
    }
}
return Controllers;
