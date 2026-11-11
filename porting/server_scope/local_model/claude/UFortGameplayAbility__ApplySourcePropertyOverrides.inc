// CONFIDENCE: LOW The thunk reads one TArray<FName> argument and writes only a bool back. The native core (0x1415eaa20) works on the ability source object (ActorInfo weak pointer at +0x18) and the current spec; its full overriding logic was not traced line by line.
// Original: exec 0x1428f65b0 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f65b0.c)
// Returns whether the core applied the overrides.
return ApplySourcePropertyOverridesCore(OverriddenPropertyNames);
