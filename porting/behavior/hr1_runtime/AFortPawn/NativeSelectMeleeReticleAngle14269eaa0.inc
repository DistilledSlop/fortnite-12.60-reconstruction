// Helper 0x14269eaa0 on AFortPawn (caller GetMeleeReticleStrikeAngle 0x142321eb0 tail-jumps here with DL preserved).
// Asm: TEST DL,DL; JZ -> MOVSS XMM0,[RCX+0x484], else MOVSS XMM0,[RCX+0x480]; RET.
// No reflected member covers 0x480/0x484, so raw float reads are used. All instructions accounted for.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(Context);
return bPrimary ? *reinterpret_cast<const float*>(Raw + 0x480) : *reinterpret_cast<const float*>(Raw + 0x484);
