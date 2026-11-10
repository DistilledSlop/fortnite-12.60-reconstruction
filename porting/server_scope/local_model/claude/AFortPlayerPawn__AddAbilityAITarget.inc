// CONFIDENCE: MEDIUM
// Original address: 0x142a7aba0 (exec thunk). Thunk reads InTarget (object) then calls FUN_142301b90(this, InTarget), captured at captures/913f6b9248d94d569e9073becc891c6a/0x142301b90.c. Full helper body is inlined below.
// Depth note: FUN_14085a990 (TArray AddUnique), FUN_1406bd0b0 (remove stale entries), FUN_140690dd0 (grow) and FUN_142ee6180 / FUN_142ee3b30 (weak pointer get/set) were read; UObject_FindFunction and FName helpers are engine helpers.
if (Role != ROLE_Authority || InTarget == nullptr)
{
	return;
}
if (InTarget->IsPendingKill())   // EInternalObjectFlags bit 29 (0x20000000) on the target's object-array entry
{
	return;
}
AbilityAITargets.AddUnique(InTarget);   // FUN_14085a990 on this->AbilityAITargets (+0x13f8)

// Bind the AITargetDied listener on the target. The function name is the segment after the last "::" in "&AFortPlayerPawn::AITargetDied".
FName AITargetDiedName = FName(TEXT("AITargetDied"));
for (const FAITargetDiedListener& Listener : InTarget->AITargetDiedListeners)   // TArray at +0x928, count +0x930, entries of 16 bytes
{
	if (Listener.Owner.Get() == this && Listener.Function == AITargetDiedName)
	{
		return;   // already registered for this pawn
	}
}
RemoveStaleAITargetDiedListeners(InTarget->AITargetDiedListeners);   // FUN_1406bd0b0

FWeakObjectPtr SelfWeak;
SelfWeak = this;   // FUN_142ee3b30
if (UObject* SelfObject = SelfWeak.Get())
{
	UObject_FindFunction(SelfObject, AITargetDiedName);   // validation only; the result is discarded in the decompile
}
InTarget->AITargetDiedListeners.Add(FAITargetDiedListener{SelfWeak, AITargetDiedName});
