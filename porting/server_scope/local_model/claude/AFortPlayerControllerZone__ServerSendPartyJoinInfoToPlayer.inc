// CONFIDENCE: HIGH (dispatch), MEDIUM (Implementation body, in deps)
// Original address: 0x142a70ce0 (exec thunk). Read order from the FFrame: RecipientId (FUniqueNetIdRepl, struct 0x1447a2020) first,
// then JoinInfoStr (FString). Reflection lists JoinInfoStr first; the thunk order is the reverse.
// Validate slot +0x20e0 -> 0x1406801d0 (STUB, return true). Implementation slot +0x20e8 -> 0x1423e9710 (dep file).
FUniqueNetIdRepl RecipientId = ReadStructParam();
FString JoinInfoStr = ReadStringParam();
ServerSendPartyJoinInfoToPlayer_Implementation(RecipientId, JoinInfoStr);
