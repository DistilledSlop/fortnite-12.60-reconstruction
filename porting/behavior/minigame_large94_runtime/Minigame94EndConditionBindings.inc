// Bindings for AFortMinigame_AddOrOverrideEndConditionForTeam.inc. Only ValidatedStatClass is implemented.
// native 0x1429d45e0 is taken to be UFortMinigameStatFilter::StaticClass() per the existing README + TSubclassOf type
// (the helper body itself was NOT decompiled in this batch); the IsChildOf check mirrors the inlined hierarchy test
// (depth at +0x38, parent array at +0x30), null on null input or failure.
static UClass* Minigame94_ValidatedStatClass(const TSubclassOf<UFortMinigameStatFilter>& Stat)
{
    UClass* Candidate = *Stat;
    return (Candidate && Candidate->IsChildOf(UFortMinigameStatFilter::StaticClass())) ? Candidate : nullptr;
}
// UNRESOLVED native helpers (no implementation, no stub semantics):
bool Minigame94_StatQueryMatches(const FMinigameEndCondition& Condition, const FFortMinigameStatQuery& Query); // 0x1420802c0
void Minigame94_EmptyTeamList(TArray<uint8>& TeamList);                                                         // 0x1420a1cb0
