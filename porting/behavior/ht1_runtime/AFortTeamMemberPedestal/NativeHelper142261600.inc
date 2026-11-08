// Helper 0x142261600 (AFortTeamMemberPedestal). Asm: CMP byte ptr [RCX+0x220],0x3 ; SETC AL ; RET (unsigned less-than).
// Caller evidence: IsInFrontend @0x144e485a0 reads weak ptr at this+0x3cc, then weak ptr at +0x30 of that object, and tail-jumps here with the second object in RCX.
// Offset evidence: no reflected member of the target class is established for +0x220. Raw uint8 access. Meaning of the state value is not established.
// CONFIDENCE: MEDIUM

return *reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(TargetObj) + 0x220) < 3;
