// CONFIDENCE: LOW
// Original: exec thunk 0x1427262d0 calls ABuildingSMActor virtual +0xa60; real body 0x141c10490.
// The real body is two virtual calls on this: +0x940 (0x141bc6310) then +0xda0 (0x141c1fd20).
RequestFullHealthVisualRefresh();
PlayFullHealthEffectsInternal();
