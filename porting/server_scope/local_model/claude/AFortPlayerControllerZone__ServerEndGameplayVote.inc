// CONFIDENCE: HIGH (dispatch), LOW (Implementation body, in deps)
// Original address: 0x142a70a50 (exec thunk). One parameter read from the FFrame: VoteType (enum 0x14292a470).
// Validate slot +0x20b0 -> 0x1406801d0 (STUB, return true). Implementation slot +0x20b8 -> 0x1423e93b0 (dep file).
EVoteType VoteType = ReadEnumParam();
ServerEndGameplayVote_Implementation(VoteType);
