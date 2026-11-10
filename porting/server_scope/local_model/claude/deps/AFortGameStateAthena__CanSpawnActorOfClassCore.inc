// Core of AFortGameStateAthena::CanSpawnActorOfClass (0x14198bb80). Member this->SpawnRules is the TArray at +0x20f8.
// Class-side cached spawn info is at UClass+0x130; if empty the class lazily builds it through virtual slot 0x390.
if (ActorClassToSpawn != nullptr)
{
	FSpawnClassInfo* ClassInfo = ActorClassToSpawn->CachedSpawnInfo;
	if (ClassInfo == nullptr)
	{
		ActorClassToSpawn->EnsureSpawnInfo();
		ClassInfo = ActorClassToSpawn->CachedSpawnInfo;
	}
	if (ClassInfo != nullptr)
	{
		return SpawnRuleCheck(SpawnRules, InstigatorActor, ClassInfo, Location, Rotation);
	}
}
FSpawnCheckResult Result;
Result.bCanSpawn = false;
Result.DebugString = FString(TEXT("Invalid_Class"));
Result.Reason = FText::GetEmpty();
return Result;
