// Behaviour snippet: UFortStoreFrontOfferInfo::GetSortPriority (body only; spliced into the generated member function).
// Original address: 0x1450b2b80 (asm: MOV RAX,[RCX+0x28]; MOV EAX,[RAX+0xe4]; RET). Thunk 0x1452f73b0 calls it directly.
// +0x28 pointer undeclared (pointee unresolved). Dword load, int32 return.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
return *reinterpret_cast<const int32*>(P + 0xe4);
