// CONFIDENCE: HIGH; native 0x142437000.
// The original validates the globals singleton even though it does not use the return value.
UFortGlobals::NativeGetGlobals();
const FGameplayTag& PartyAssist = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x1f30);
return GameplayTags.HasTagExact(PartyAssist);
