// Original body address: 0x141d813f0 (thunk 0x142772950)
// Offset evidence: +0x20 is UObject::OuterPrivate (engine storage below the class own region, min 0x350), read raw.
// The asm then tests the Outer object IsA USkeletalMeshComponent::StaticClass() and returns it, else null.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
UObject* Outer = *reinterpret_cast<UObject* const*>(Raw + 0x20);
if (Outer == nullptr)
{
	return nullptr;
}
if (!Outer->IsA(USkeletalMeshComponent::StaticClass()))
{
	return nullptr;
}
return static_cast<USkeletalMeshComponent*>(Outer);
