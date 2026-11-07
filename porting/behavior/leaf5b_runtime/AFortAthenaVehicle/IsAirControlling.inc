// Thunk 0x14283be20: no ADD RCX (thunk_delta 0x0), virtual call [RAX+0x8e0] on this (wrapper_delta 0).
// Body 0x14084cc60: MOVZX EAX,byte [RCX+0x448]; AND AL,1; RET. Object offset = 0x448, bit 0 (mask 0x1).
// Byte 0x448 is owned by bAllowInteractBetweenFortVolumes_Runtime (mask 0x10); no declared bool has mask 0x1 there, so raw bit access.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return (Raw[0x448] & 0x1) != 0;
