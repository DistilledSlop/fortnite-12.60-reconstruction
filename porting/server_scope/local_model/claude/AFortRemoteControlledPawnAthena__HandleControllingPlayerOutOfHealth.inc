// CONFIDENCE: MEDIUM. Exec thunk with no parameters: forwards to the native helper with the pawn as implicit context (RCX passed through; the only frame step is the P_FINISH counter, ignored).
// Original address: 0x142ac24a0 (exec). Helper: 0x14243a470 (deps AFortRemoteControlledPawnAthena__HandleControllingPlayerOutOfHealth__Helper.json).
HandleControllingPlayerOutOfHealthHelper();
