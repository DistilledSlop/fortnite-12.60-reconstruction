// Helper 0x142099610 on AFortMinigame (caller CheckEndConditionsInternal 0x142087920).
// Asm: gate byte [this+0x75d]; call 0x142087960 (CheckRemainingLivingPlayersForEndCondition) true -> true;
// loop EndConditionArray (Data [+0x678], Num [+0x680], stride 0x30) calling EvaluateStatQuery(elem, -1) -> true on first true; else false.
// Argument order follows the ABI (RDX = element pointer, R8 = -1); the reflection hint lists params in a different order.
// Null guard on Context added; every other instruction is accounted for.
// CONFIDENCE: MEDIUM
AFortMinigame* Minigame = Cast<AFortMinigame>(Context);
if (Minigame == nullptr)
{
	return false;
}
if (*(reinterpret_cast<const uint8*>(Minigame) + 0x75d) == 0)
{
	return false;
}
if (Minigame->CheckRemainingLivingPlayersForEndCondition())
{
	return true;
}
for (FMinigameEndCondition& Cond : Minigame->EndConditionArray)
{
	if (Minigame->EvaluateStatQuery(Cond, -1))
	{
		return true;
	}
}
return false;
