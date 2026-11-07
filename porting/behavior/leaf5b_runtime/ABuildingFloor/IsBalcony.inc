// Thunk 0x1427403a0: no ADD RCX (thunk_delta 0x0), CALL 0x141be2510 directly (wrapper_delta 0).
// Body 0x141be2510: MOV EAX,dword [RCX+0xb30]; SHR EAX,1; AND AL,1; RET. Object offset = 0x0 + 0xb30 = 0xb30.
// Byte 0xb30 is owned by the header bitfield bShouldIgnoreForHorizontalHotspotSearch:1 (bit 0, mask 0x1; bool_bits.json).
// Bit 1 (mask 0x2) of the same byte has no declared owner, so it is read by raw offset from this.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return (Raw[0xb30] & 0x2) != 0;
