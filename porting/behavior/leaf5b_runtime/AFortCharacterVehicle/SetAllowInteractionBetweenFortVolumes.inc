// Thunk 0x142862090: asm read. Stack argument read via the exec helpers, normalises the bool with SETNZ DL, then
//   ADD RCX,0x1048 (thunk_delta 0x1048) and virtual call [RAX+0xc8] (wrapper_delta 0).
// Body 0x141963c60: MOV byte [RCX+0x20d8],DL; RET. Object offset = 0x1048 + 0x20d8 = 0x3120 (same bool as the getter).
// CONFIDENCE: HIGH
bAllowInteractBetweenFortVolumes = bInteractBetweenVolumes;
