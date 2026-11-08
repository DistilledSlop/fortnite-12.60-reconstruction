// Behaviour snippet: UCustomCharacterPartAnimInstance::GetAssociatedPlayerPawn (body only).
// Original address: 0x141d7bc20 (thunk 0x142772870)
// Offset evidence: no reflected member access. Calls: 0x143fccf40 = UCustomCharacterPartAnimInstance::NativeGetAssociatedPawnFromOuter143fccf40 (ported static helper, receiver this); 0x142a75f10 = AFortPlayerPawn::StaticClass(). The ClassPrivate (+0x10) and ancestor-depth compare is UObject::IsA; the body returns the helper object when it is an AFortPlayerPawn, else null.
// CONFIDENCE: HIGH
static_assert(sizeof(AFortPlayerPawn) > 0, "AFortPlayerPawn must be complete for StaticClass");
UObject* AssociatedObject = UCustomCharacterPartAnimInstance::NativeGetAssociatedPawnFromOuter143fccf40(const_cast<UCustomCharacterPartAnimInstance*>(this));
if (AssociatedObject == nullptr)
{
	return nullptr;
}
if (!AssociatedObject->IsA(AFortPlayerPawn::StaticClass()))
{
	return nullptr;
}
return reinterpret_cast<AFortPlayerPawn*>(AssociatedObject);
