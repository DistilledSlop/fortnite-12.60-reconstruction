// CONFIDENCE: HIGH
// Original: exec thunk 0x142a884f0. Reflection params: A (Struct), B (Struct). The thunk reads A first, then B,
// and calls the comparison helper 0x140d7c5a0 (dep: deps/AFortPlayerState__AreUniqueIDsIdentical__UniqueNetIdsEqual.json) with (A, B).
// The comparison is symmetric, so the argument order does not change the result.
return UniqueNetIdsEqual(A, B);
