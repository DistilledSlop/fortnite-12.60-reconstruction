// Original body 0x141b720c0: MOVZX EAX,byte [RCX+0xc8]; RET.
// Receiver: exec thunk 0x1427cfd10 does ADD RCX,0x3b0, then the vtable slot is the adjustor 0x1419dd170 (ADD RCX,0xc40; JMP 0x141b720c0),
// so the byte lives at this+0x3b0+0xc40+0xc8 = this+0x10b8 (unreflected, declared as a recovered member).
// CONFIDENCE: MEDIUM
return NativeRecoveredBool_0x10b8;
