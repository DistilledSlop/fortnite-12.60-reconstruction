// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x142834990. The thunk reads FortPC (offset 0) and
// ItemsToSpawn (offset 8) from bytecode, then calls Stub_Empty() and releases the temporary array.
// The native body is therefore empty: no analytics are emitted from native code.
// The analytics event is raised on the Blueprint side.
