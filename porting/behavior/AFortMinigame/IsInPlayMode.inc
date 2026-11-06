// Original address: 0x1429c2d50.
// Decompile source: porting/generated/decomp/AFortMinigame/IsInPlayMode__1429c2d50.c
// Inferred: direct body (no tail jump): returns 0 only when both byte flags at +0xc61 and +0xc60 are zero, else 1. Both flags are unnamed in reflection and are declared as member_field storage (NativeRecoveredBool_0xc60, NativeRecoveredBool_0xc61).
// CONFIDENCE: HIGH
return NativeRecoveredBool_0xc61 || NativeRecoveredBool_0xc60;
