// CONFIDENCE: MEDIUM. Original exec 0x14296e4d0; body 0x141ef4be0. Walks the world's player controller list and collects local FortPlayerControllers.
// A non-spectating local controller is added when bIncludeNonSpectators is set; a spectating one when bIncludeSpectators is set.
TArray<AFortPlayerController*> LocalControllers;

UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
if (World == nullptr)
{
	return LocalControllers;
}

for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
{
	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(It->Get());
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		continue;
	}

	const bool bIsSpectating = PlayerController->IsA(AFortPlayerControllerSpectating::StaticClass());
	const bool bInclude = bIsSpectating ? bIncludeSpectators : bIncludeNonSpectators;
	if (bInclude)
	{
		LocalControllers.Add(PlayerController);
	}
}

return LocalControllers;
