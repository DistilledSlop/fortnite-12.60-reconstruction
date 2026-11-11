// CONFIDENCE: MEDIUM Same container apply as ApplyGameplayEffectContainer. The native wrapper 0x1415ea9b0 forwards the level to the same virtual core (slot 0x480).
// Original: exec 0x1428f63b0 (decompile captures/0da226f5ed8442fb9ce91e4985f6c96a/0x1428f63b0.c)
// StatName is a CSV profiling label only; it does not change gameplay results.
const TArray<FActiveGameplayEffectHandle> Handles = ApplyGameplayEffectContainerCore(TargetData, ApplicationTag, GameplayEffectLevel);
(void)StatName; // CSV label consumed by the profiler wrapper, no gameplay effect
return Handles;
