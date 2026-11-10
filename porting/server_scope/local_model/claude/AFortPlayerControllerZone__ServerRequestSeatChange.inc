// CONFIDENCE: HIGH (dispatch), MEDIUM (Implementation body, in deps)
// Original address: 0x142a70b20 (exec thunk). One parameter: TargetSeatIndex (int32, offset 0).
// Validate slot +0x2050 -> 0x1406801d0 (STUB, return true). Implementation slot +0x2058 -> 0x1423e9500 (dep file).
int32 TargetSeatIndex = ReadIntParam();
ServerRequestSeatChange_Implementation(TargetSeatIndex);
