// CONFIDENCE: MEDIUM Thunk reads TargetData (handle copy, shared references), ApplicationTag, GameplayEffectLevel from the FFrame, calls the container core through vtable slot 0x480 (resolves to 0x1415ea920), and returns the handle array.
// Original: exec 0x1428f61d0 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f61d0.c)
// The native copy-and-release of the handle is the C++ pass-by-value copy here.
const TArray<FActiveGameplayEffectHandle> Handles = ApplyGameplayEffectContainerCore(TargetData, ApplicationTag, GameplayEffectLevel);
return Handles;
