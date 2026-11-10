// Native 0x14198ec00: walks the spawn rule array (this->SpawnRules at +0x20f8) and asks each rule through virtual slot 0x48 whether the spawn is allowed. The first refusal is returned with that rule's reason; if all allow it, the result is success with an empty reason.
// LOW on the element type: the rule class is not named in the captures; the call is named CheckSpawn.
FSpawnCheckResult Result;
for (const FSpawnRuleEntry& Entry : SpawnRules)
{
	ISpawnRule* Rule = Entry.IsValid() ? Entry.Rule : nullptr;
	bool bAllowed = false;
	FText RuleReason;
	Rule->CheckSpawn(bAllowed, RuleReason, InstigatorActor, ClassInfo, Location, Rotation);
	if (!bAllowed)
	{
		Result.bCanSpawn = false;
		Result.Reason = RuleReason;
		return Result;
	}
}
Result.bCanSpawn = true;
Result.Reason = FText::GetEmpty();
return Result;
