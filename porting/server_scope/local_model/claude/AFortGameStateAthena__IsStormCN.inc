// CONFIDENCE: MEDIUM. Exec thunk 0x142920160 tail-jumps to the same body as 0x14199e4e0 (thunk_FUN_14199e4e0). The binary folds the identical CN check, so this forwards to the same shared helper (deps AFortGameStateAthena__IsCNModeActive). Name suggests a storm check, but the folded body does not read storm state directly.
// Original address: 0x142920160 (exec).
return IsCNModeActive();
