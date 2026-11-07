// Thunk 0x145199140: asm file not in porting/generated/native*; checked the Ghidra decompile
//   porting/generated/decomp/UAthenaEventMatchScoreboardScreenBase/HandleSquadEliminated__145199140.c: the thunk calls body with the
//   unchanged RCX (this), so thunk_delta 0x0 and wrapper_delta 0.
// Body 0x1429e7b90: MOV byte [RCX+0x491],1; RET. Object offset = 0x491, in the unreflected gap 0x490..0x497
//   (SquadDataArray 0x480 TArray ends 0x490; ScoreboardListView at 0x498). Member NativeRecoveredBool_0x491 (new member_field).
// CONFIDENCE: MEDIUM
NativeRecoveredBool_0x491 = true;
