// Thunk 0x1451991c0: asm file not in porting/generated/native*; checked the Ghidra decompile
//   porting/generated/decomp/UAthenaEventMatchScoreboardScreenBase/HandleSquadKillScoreChanged__1451991c0.c: args read by the exec helpers, then the
//   body is called with the unchanged RCX (this): thunk_delta 0x0, wrapper_delta 0.
// Body 0x1429e7b90 (shared with HandleSquadEliminated): MOV byte [RCX+0x491],1; RET. Object offset 0x491 -> member NativeRecoveredBool_0x491.
// CONFIDENCE: MEDIUM
NativeRecoveredBool_0x491 = true;
