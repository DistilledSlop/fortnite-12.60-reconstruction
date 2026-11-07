// Thunk 0x14280d330: saves this in RBP, reads the PlayerController argument with the standard exec helpers (0x142ed9cf0 / 0x142ed9d20),
//   then LEA RCX,[RBP+0x310] (thunk_delta 0x310) and virtual call [RAX+0x10] on the sub-object (wrapper_delta 0).
// Body 0x1416a1300: MOVZX EAX,byte [RCX+0x80]; RET. Object offset = 0x310 + 0x80 = 0x390.
// 0x390 is reflected InfiltrationTeamInfos[2] (FInfiltrationTeamInfo, size 12 per type_closure.json); byte +0 of element 0 is TeamNum (uint8).
// CONFIDENCE: HIGH
return InfiltrationTeamInfos[0].TeamNum;
