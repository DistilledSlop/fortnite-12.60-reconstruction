// CONFIDENCE: HIGH (dispatch), LOW (Implementation body, in deps)
// Original address: 0x142a70870 (exec thunk). Read order from the FFrame: VoteType (enum byte) first, then VoteConfig (struct 0x14292a500).
// The thunk pre-fills VoteConfig with its defaults (45.0f duration and flag words) before the struct read.
// Validate slot +0x20d0 -> 0x1406801d0 (STUB, return true). Implementation slot +0x20d8 -> 0x1423e8db0 (dep file).
EVoteType VoteType = ReadEnumParam();
FVoteConfig VoteConfig;   // default-constructed, then overwritten from the FFrame
ReadStructParam(VoteConfig);
ServerBeginGameplayVote_Implementation(VoteType, VoteConfig);
