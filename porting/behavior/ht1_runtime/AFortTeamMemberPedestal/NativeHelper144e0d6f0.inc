// Helper 0x144e0d6f0 (AFortTeamMemberPedestal). Asm: CMP dword ptr [RCX+0x21c],0x0 ; SETLE AL ; RET (signed).
// Caller evidence: IsInLocalSquadGroup @0x144e485f0 passes the reflected OwningPedestalGroup (this+0x380, Object) in RCX; it returns true when that is null, else this helper result.
// Offset evidence: no reflected member of the group class is established for +0x21c. Raw int32 access. Meaning is not established.
// CONFIDENCE: MEDIUM

return *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(GroupObj) + 0x21c) <= 0;
