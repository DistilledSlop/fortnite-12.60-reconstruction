// Behaviour snippet: UFortStoreFrontOfferInfo::GetNumRequirements (body only; spliced into the generated member function).
// Original address: 0x1450b08e0 (asm: MOV RAX,[RCX+0x28]; MOV EAX,[RAX+0x250]; RET). Thunk 0x1452f6a20 calls it directly.
// +0x28 pointer undeclared in UFortStoreFrontOfferInfo.h (only OfferDisplayAsset at 0x40 reflected); pointee unresolved.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
return *reinterpret_cast<const int32*>(P + 0x250);
