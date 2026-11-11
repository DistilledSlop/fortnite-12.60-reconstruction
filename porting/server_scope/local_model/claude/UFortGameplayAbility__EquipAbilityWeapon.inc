// CONFIDENCE: HIGH Native body is empty: the thunk reads ItemLevel and WeaponDefinition from the FFrame and then calls Stub_Empty(), a no-op. Empty by design in 12.60; the parameters are accepted and ignored.
// Original: exec 0x1427630e0 (decompile captures/a1af586513d34254a090366c1f3f8382/0x1427630e0.c)
(void)ItemLevel;
(void)WeaponDefinition;
