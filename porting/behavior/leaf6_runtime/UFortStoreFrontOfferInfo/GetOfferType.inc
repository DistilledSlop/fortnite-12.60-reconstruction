// Behaviour snippet: UFortStoreFrontOfferInfo::GetOfferType (body only; spliced into the generated member function).
// Original address: 0x1450b0d10 (asm: MOV RAX,[RCX+0x28]; MOVZX EAX,byte [RAX+0x60]; RET). Thunk 0x1452f6b70 calls it directly.
// Return enum ECatalogOfferType (reflection enum VA 0x1411411e0, UnderlyingType Byte). The enum has no declaration in native/Source headers.
// +0x28 pointer undeclared; pointee unresolved. Byte load zero-extended, so the uint8 underlying value is cast to the enum.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0x28);
return static_cast<ECatalogOfferType>(*reinterpret_cast<const uint8*>(P + 0x60));
