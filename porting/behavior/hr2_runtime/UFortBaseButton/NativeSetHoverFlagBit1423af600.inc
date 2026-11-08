// Helper 0x1423af600 (host UFortBaseButton), tail-called from UFortBaseButton::SetHovered (body 0x144d95470).
// The caller passes RCX = *(weak target this+0x7d8 ->  + 0x430), DL = the flag (MOVZX from its bool argument).
// Asm: AND byte ptr [RCX+0x28],0xfe ; OR byte ptr [RCX+0x28],DL ; RET
// Result: bit 0 of the byte at +0x28 is replaced by Value; the other bits are kept.
// The owning class of the target object is not identified by reflection (the +0x28 lookup resolves to UWidget.Slot, which does not apply to this sub-object), so raw access is used.
uint8* FlagBytes = reinterpret_cast<uint8*>(TargetObj) + 0x28;
*FlagBytes = static_cast<uint8>((*FlagBytes & 0xfe) | Value);
// CONFIDENCE: MEDIUM
