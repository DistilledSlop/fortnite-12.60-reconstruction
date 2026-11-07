// Thunk 0x1427cfd10: ADD RCX,0x3b0 (thunk_delta 0x3b0), then virtual call [RAX+0x768] on the sub-object.
// Vtable adjustor wrapper 0x1418f1260: ADD RCX,0xc90; JMP 0x141b720c0 (wrapper_delta 0xc90). Total delta 0x1040.
// Body 0x141b720c0: MOVZX EAX,byte [RCX+0xc8]; RET. Object offset = 0x1040 + 0xc8 = 0x1108.
// 0x1108 lies in Doghouse-inherited storage (between reflected BoostAction at 0x1040 and AileronRoll at 0x1118), not in Ferret's own region (starts 0x15a8).
// No reflected member covers it, so it is a raw-offset access from this.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return *reinterpret_cast<const bool*>(Raw + 0x1108);
