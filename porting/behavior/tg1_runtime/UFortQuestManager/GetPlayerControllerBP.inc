// Original body 0x142441230 (thunk 0x142ab8c70). Evidence: raw object pointer at this+0x20 (no reflected member), IsA UFortRegisteredPlayerInfo gate, then tail-jump to NativeHelper141f60080 with that same pointer (ABI argument; the helper's declared parameter type differs, passed by reinterpret_cast as the asm does).
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
UObject* InfoObj = *reinterpret_cast<UObject* const*>(Raw + 0x20);
if (InfoObj == nullptr)
{
	return nullptr;
}
static_assert(sizeof(UFortRegisteredPlayerInfo) > 0, "UFortRegisteredPlayerInfo must be complete for the IsA gate.");
if (!InfoObj->IsA(UFortRegisteredPlayerInfo::StaticClass()))
{
	return nullptr;
}
return reinterpret_cast<AFortPlayerController*>(UFortQuestManager::NativeHelper141f60080(reinterpret_cast<UFortQuestManager*>(InfoObj)));
