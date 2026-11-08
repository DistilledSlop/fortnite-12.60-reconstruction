// Helper 0x1425702e0 (AFortPlayerControllerOutpost). Asm: MOVZX EAX,byte [RCX+0x330]; AND AL,0x1; RET.
// Caller evidence: IsOutpostOwner @0x14254c3f0 passes its controller in RCX and tests AL.
// Offset caveat: reflection lists APlayerController +0x330 as CheatManager (Object), so this bit read overlaps a reflected pointer. bool_bits.json lists outpost bools at 0x2c5c/0x2c70, not 0x330. The asm bit is reproduced as read.
// CONFIDENCE: MEDIUM

return (*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(ControllerObj) + 0x330) & 0x1) != 0;
