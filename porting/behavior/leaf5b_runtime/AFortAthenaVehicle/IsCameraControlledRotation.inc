// Thunk 0x14283bec0: no ADD RCX (thunk_delta 0x0), CALL 0x14191bbc0 directly (wrapper_delta 0).
// Body 0x14191bbc0: MOV RAX,qword [RCX+0x738]; MOVZX EAX,byte [RAX+0x335]; RET. Object offset of the pointer slot = 0x738.
// 0x738 is in the Vehicle own region, between OnNotOutOfFuel (0x728, 16 bytes) and OnMountedWeaponFiredEvent (0x748): unreflected.
// The pointee class is unresolved, so the pointer slot and the pointee byte (+0x335) are both raw-offset accesses.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const uint8* P = *reinterpret_cast<const uint8* const*>(Raw + 0x738);
return *reinterpret_cast<const bool*>(P + 0x335);
