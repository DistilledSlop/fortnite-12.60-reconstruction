// Original body address: 0x141641350 (helper, no thunk)
// Offset evidence: asm loads the raw object pointer at [RCX + 0x130]; no reflected member of UCreativeQuestComponent covers +0x130.
// Asm: null in gives null out. Otherwise the object's class is tested against AFortGameStateAthena::StaticClass() (0x14290f690, hinted) and the pointer is returned only on a match.
// Callers (6) include ABuildingFoundation::OnLevelShown (0x141be38f0) and UCreativeQuestComponent::RegisterQuest (0x141d8ea00).
// CONFIDENCE: HIGH
UObject* GameStateCandidate = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(QuestComponent) + 0x130);
if (GameStateCandidate == nullptr || !GameStateCandidate->IsA(AFortGameStateAthena::StaticClass()))
{
	return nullptr;
}
return GameStateCandidate;
