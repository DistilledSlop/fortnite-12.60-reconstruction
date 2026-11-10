// CONFIDENCE: LOW. Original 0x14292d2b0. The thunk reads LootLevel and calls thunk 0x141eb0560 (same body as 0x143f7a920). That callee takes only the object, so LootLevel is not consumed and the result is discarded.
// Original address: 0x14292d2b0 (exec).
(void)LootLevel;
ResolveMissionRewardLootLevelOverride();
