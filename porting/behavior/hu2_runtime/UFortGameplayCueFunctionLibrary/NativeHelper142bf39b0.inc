// Helper 0x142bf39b0 (static on UFortGameplayCueFunctionLibrary).
// Evidence: porting/generated/native-cap-helpers3/0x142bf39b0.asm, 2 instructions: MOVSD XMM0,[RCX+0x50]; RET.
// Caller evidence: 0x142ed3b30 passes the object returned by 0x142bee950 in RCX and subtracts this double from another.
// No reflected member of the caller class is established at +0x50, so the read is raw with the asm width (double).
// CONFIDENCE: HIGH
return *reinterpret_cast<const double*>(reinterpret_cast<const uint8*>(Context) + 0x50);
