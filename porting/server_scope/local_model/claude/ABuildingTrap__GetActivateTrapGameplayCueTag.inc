// CONFIDENCE: HIGH - returns the native gameplay cue tag "GameplayCue.Abilities.Activation.Traps.ActivateTrap".
// Original helper 0x141c38300 reads the FFortNativeGameplayTagsRuntime singleton (0x141ed1800) field at original offset 0x628; the registered tag is "GameplayCue.Abilities.Activation.Traps.ActivateTrap" (FortNativeGameplayTagsRuntime.h, Register(0x628, ...)).
return FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Abilities.Activation.Traps.ActivateTrap")));
