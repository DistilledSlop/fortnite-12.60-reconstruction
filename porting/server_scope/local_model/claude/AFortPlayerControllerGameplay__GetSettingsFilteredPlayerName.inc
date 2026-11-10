// CONFIDENCE: MEDIUM
// Original address: 0x142a65070 (exec thunk). Parameter OtherPlayerState is read from the FFrame (class 0x142a85440).
// The result FString is produced by the virtual filter at slot +0x1fc8 (0x1423d5350), called with bRespectSettings = true
// (the thunk passes R9B = 1). The filter writes the name into a temporary and returns that temporary.
FString Result;
FilterPlayerNameForSettings(Result, OtherPlayerState, /*bRespectSettings*/ true);
return Result;
