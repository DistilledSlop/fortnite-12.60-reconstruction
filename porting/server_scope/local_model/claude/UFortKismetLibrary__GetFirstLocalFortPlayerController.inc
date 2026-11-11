// CONFIDENCE: HIGH. Original exec 0x14296d400; body 0x141ef2890. Walks the world's player controller list (world +0x1c8).
// A local FortPlayerController is accepted when it is non-spectating and bIncludeNonSpectators is set, or spectating and bIncludeSpectators is set.
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
if (World == nullptr)
{
	return nullptr;
}

for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
{
	AFortPlayerController* PlayerController = Cast<AFortPlayerController>(It->Get());
	if (PlayerController == nullptr || !PlayerController->IsLocalController())
	{
		continue;
	}

	const bool bIsSpectating = PlayerController->IsA(AFortPlayerControllerSpectating::StaticClass());
	const bool bAccept = bIsSpectating ? bIncludeSpectators : bIncludeNonSpectators;
	if (bAccept)
	{
		return PlayerController;
	}
}

return nullptr;
