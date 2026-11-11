// CONFIDENCE: MEDIUM. Original exec 0x14296db10; body 0x141ef39b0. Reads the world's AuthorityGameMode (world +0x128), requires AFortGameMode, then copies its game-context tags (virtual slot 0x980) into the out container.
// Returns false and leaves the container untouched when there is no world or no AFortGameMode.
if (WorldContextObject == nullptr)
{
	return false;
}

UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
AFortGameMode* GameMode = World != nullptr ? Cast<AFortGameMode>(World->AuthorityGameMode) : nullptr;
if (GameMode == nullptr)
{
	return false;
}

OutGameContextGameplayTags = GameMode->GetGameContextGameplayTags();
return true;
