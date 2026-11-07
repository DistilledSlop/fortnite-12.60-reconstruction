// Original body: 0x1423061d0 (thunk 0x142a7ae10)
// Offset evidence: CMP byte [RCX+0xf0],3 (Role); JNZ skips MOV [RCX+0x2284],DL; MOV [RCX+0x2285],DL always
// CONFIDENCE: HIGH
if (static_cast<uint8>(Role) == 3)
{
    bParachuteLockedOpen = bLocked;
}
bLocalParachuteLockedOpen = bLocked;
