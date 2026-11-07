// Behaviour snippet: UFortStoreFrontOfferInfo::IsRefundable (body only; spliced into the generated member function).
// Original address: 0x1450b9920 (asm: MOV RAX,[RCX+0x28]; MOVZX EAX,byte [RAX+0x258]; RET). Thunk 0x1452f7dc0 calls it directly.
// +0x28 pointer undeclared (pointee unresolved). Byte load (bool), no null check in asm.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
return *reinterpret_cast<const bool*>(P + 0x258);
