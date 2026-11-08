// Native helper 0x141641300 (static helper on AFortMinigame; callers ABuildingProp, AFortAIDirector, AFortAthenaAIBotController, AFortMinigame).
// Asm: RBX = [RCX+0x128]; null returns null; CALL AFortGameModeAthena::StaticClass (0x1428ee970); then the inline IsA test of RBX
// through its class at [RBX+0x10]; on success returns RBX, else null.
// Reflection lists AActor::Children (0x120, TArray) as spanning 0x128, but the asm loads an 8-byte pointer there, so a raw load is kept.
// CONFIDENCE: MEDIUM
UObject* const Candidate = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(MinigameObject) + 0x128);
if (Candidate != nullptr && Candidate->IsA(AFortGameModeAthena::StaticClass()))
{
	return Candidate;
}
return nullptr;
