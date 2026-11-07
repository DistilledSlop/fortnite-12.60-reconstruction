// Thunk 0x1427d7f50: ADD RCX,0x1118 (thunk_delta 0x1118), CALL 0x141b720c0 directly (wrapper_delta 0).
// Body 0x141b720c0: MOVZX EAX,byte [RCX+0xc8]; RET. Object offset = 0x1118 + 0xc8 = 0x11e0.
// 0x11e0 is inside reflected struct AileronRoll (Doghouse +0x1118, size 0xe0; next member at 0x11f8). FAileronRoll.Action is at struct +0 and Direction at +0xd8.
// Byte +0xc8 of the struct lies in Action (FFortRechargingActionTimer), whose bool fields have no reflected offsets: raw-offset access.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return *reinterpret_cast<const bool*>(Raw + 0x11e0);
