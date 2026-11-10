// CONFIDENCE: HIGH - returns the native gameplay cue tag "GameplayCue.Abilities.Activation.Traps.DelayBegin".
// Original helper 0x141c398e0 reads the FFortNativeGameplayTagsRuntime singleton (0x141ed1800) field at original offset 0x620; the registered tag is "GameplayCue.Abilities.Activation.Traps.DelayBegin" (FortNativeGameplayTagsRuntime.h, Register(0x620, ...)).
return FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Abilities.Activation.Traps.DelayBegin")));
