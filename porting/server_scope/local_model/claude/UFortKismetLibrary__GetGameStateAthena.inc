// CONFIDENCE: HIGH. Original exec 0x14296dc10; body 0x141ef3a40. Returns the world's game state when it is an AFortGameStateAthena, otherwise null.
if (WorldContextObject == nullptr)
{
	return nullptr;
}

UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
if (World == nullptr)
{
	return nullptr;
}

return Cast<AFortGameStateAthena>(World->GameState);
