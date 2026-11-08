// Helper body for AFortPlayerPawn static helper at 0x144517e00 (asm: MOV byte ptr [RCX + 0x6f6],0x2; RET). No branches, no calls.
// Caller: AFortPlayerPawn::ResetClothAndDynamics 0x142349360 loops over a pawn array and calls this with RCX = pawn.
// Offset evidence: lookup_offset AFortPlayerPawn 0x6f6 reports no member. AFortPawn reflects DBNOLandingSound (Object) at +0x6f0,
// whose 8-byte span includes +0x6f6, so reflection and this store disagree. The native byte store is reproduced as-is; the meaning of value 2 is unknown from this helper alone.
// CONFIDENCE: MEDIUM
*reinterpret_cast<uint8*>(reinterpret_cast<uint8*>(TargetPawn) + 0x6f6) = 2;
