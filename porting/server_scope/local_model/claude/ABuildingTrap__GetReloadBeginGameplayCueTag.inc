// CONFIDENCE: HIGH - returns the native gameplay cue tag "GameplayCue.Abilities.Activation.Traps.ReloadBegin".
// Original helper 0x141c3c940 reads the FFortNativeGameplayTagsRuntime singleton (0x141ed1800) field at original offset 0x630; the registered tag is "GameplayCue.Abilities.Activation.Traps.ReloadBegin" (FortNativeGameplayTagsRuntime.h, Register(0x630, ...)).
return FGameplayTag::RequestGameplayTag(FName(TEXT("GameplayCue.Abilities.Activation.Traps.ReloadBegin")));
