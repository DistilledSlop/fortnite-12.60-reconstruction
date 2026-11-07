// Behaviour snippet: UAthenaHUDContext::HasLockOnTarget (body only; spliced into the generated member function).
// Original address: 0x144c980e0 (thunk 0x14519f1d0)
// Offset evidence: this+0x20 lies below the class's lowest own reflected offset (0x38), so it is inherited engine storage read as a raw object pointer (rule 3). The body checks Obj->IsA(ULocalPlayer) and Obj->IsA(AFortPlayerController), then calls 0x1406801e0, which is XOR AL,AL; RET (constant false). The TEST AL,AL / JZ path therefore always yields false.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
UObject* Obj = *reinterpret_cast<UObject* const*>(Raw + 0x20);
if (Obj == nullptr || !Obj->IsA(ULocalPlayer::StaticClass()) || !Obj->IsA(AFortPlayerController::StaticClass()))
{
	return false;
}
// The call to 0x1406801e0 (constant false, no side effects) is folded: the TEST AL,AL branch never takes the true path.
return false;
