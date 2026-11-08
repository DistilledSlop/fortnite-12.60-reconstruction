// Helper 0x1407327b0, host UCreativeQuestComponent (static). 24 instructions.
// Both arguments are 8-byte TWeakObjectPtr records: int32 ObjectIndex at +0x0, int32 ObjectSerialNumber at +0x4.
// Asm: equal index and serial -> true; otherwise true only when neither weak pointer IsValid (0x142ee7880 = TWeakObjectPtr::IsValid, called on A then B).
// Callers: UCreativeQuestComponent::RegisterQuest (0x141d8ea00) and UnregisterQuest (0x141d95580) pass RCX = array element address, RDX = address of a local weak record.
// CONFIDENCE: HIGH
const int32* WeakA = reinterpret_cast<const int32*>(A);
const int32* WeakB = reinterpret_cast<const int32*>(B);
if (WeakA[0] == WeakB[0] && WeakA[1] == WeakB[1])
{
	return true;
}
if (reinterpret_cast<const TWeakObjectPtr<UObject>*>(A)->IsValid())
{
	return false;
}
if (reinterpret_cast<const TWeakObjectPtr<UObject>*>(B)->IsValid())
{
	return false;
}
return true;
