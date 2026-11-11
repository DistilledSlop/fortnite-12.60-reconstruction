// CONFIDENCE: HIGH. Returns an empty array. Same as DropInstancedLoot: the native exec reads its arguments and calls the zero-initialiser at 0x140738030, which returns an empty TArray, so no loot is dropped at DropLocation.
// Original address: 0x142969820 (exec). Native helper: 0x140738030 (zero-initialises the returned TArray).
return {};
