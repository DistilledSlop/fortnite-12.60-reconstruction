// CONFIDENCE: HIGH - BlueprintNativeEvent with an empty native default.
// Original addresses: 0x14275c6e0 (exec thunk)
// Exec 0x14275c6e0 zeroes the out struct and calls vtable slot 0xea8 on this, which resolves to 0x1406801c0 (Stub_Empty, a no-op).
// Blueprint overrides supply the values; the native default leaves OutVariables default-initialised.
OutVariables = decltype(OutVariables){};
