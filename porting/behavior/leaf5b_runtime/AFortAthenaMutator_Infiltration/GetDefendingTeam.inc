// Thunk 0x14280d3d0: same shape as GetAttackingTeam; LEA RCX,[RBP+0x310] (thunk_delta 0x310), virtual call [RAX+0x18] (wrapper_delta 0).
// Body 0x141b01d50: MOVZX EAX,byte [RCX+0x8c]; RET. Object offset = 0x310 + 0x8c = 0x39c = 0x390 + 12.
// 0x39c is byte +0 of InfiltrationTeamInfos[1] (sizeof FInfiltrationTeamInfo = 12), i.e. TeamNum of element 1.
// CONFIDENCE: HIGH
return InfiltrationTeamInfos[1].TeamNum;
