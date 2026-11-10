// CONFIDENCE: HIGH (dispatch), LOW (Implementation body, in deps)
// Original address: 0x142a70f50 (exec thunk). Read order from the FFrame: NewPlayerToSpectate (object, class 0x1447ada70) first,
// then bAllowStateChange (bool). Reflection lists bAllowStateChange first; the thunk order is the reverse.
// Validate slot +0x2148 -> 0x1406801d0 (STUB, return true). Implementation slot +0x2150 -> 0x1423e9920 (dep file).
APlayerState* NewPlayerToSpectate = ReadObjectParam();
bool bAllowStateChange = ReadBoolParam();
ServerSpectatePlayer_Implementation(NewPlayerToSpectate, bAllowStateChange);
