#include "FortNativeGameplayTagsRuntime.h"
#include "GameplayTagsManager.h"

#if !defined(__cpp_threadsafe_static_init)
#error "Original native gameplay tag singleton requires thread-safe local statics"
#endif

FFortNativeGameplayTagsRuntime& FFortNativeGameplayTagsRuntime::Get()
{
    static FFortNativeGameplayTagsRuntime Instance;
    return Instance;
}

const FGameplayTag& FFortNativeGameplayTagsRuntime::GetTagAtOriginalOffset(uint32 Offset) const
{
    checkf((Offset & 7u) == 0 && Offset < 0x2568 &&
        !(Offset >= 0x880 && Offset < 0x920) &&
        !(Offset >= 0x1b10 && Offset < 0x1b50) &&
        !(Offset >= 0x2058 && Offset < 0x2148),
        TEXT("Offset 0x%x is not an original native gameplay tag field"), Offset);
    return TagsByOriginalOffset[Offset / 8];
}

void FFortNativeGameplayTagsRuntime::Register(uint32 Offset, const ANSICHAR* Name)
{
    FGameplayTag& Tag = TagsByOriginalOffset[Offset / 8];
    Tag = UGameplayTagsManager::Get().AddNativeGameplayTag(FName(Name), TEXT("(Native)"));
    // Original producer 0x141ec98b0 inserts even an invalid returned tag.
    if (FCStringAnsi::Strnicmp(Name, "GameplayCue", 11) == 0)
    {
        GameplayCueTags20f8.Add(Tag);
    }
}

void FFortNativeGameplayTagsRuntime::RegisterMovement(
    uint32 Offset, const ANSICHAR* Name, uint8 Value, bool bCustom)
{
    Register(Offset, Name);
    (bCustom ? CustomMovementModeTags8d0 : MovementModeTags880).Add(Value, TagsByOriginalOffset[Offset / 8]);
}

FFortNativeGameplayTagsRuntime::FFortNativeGameplayTagsRuntime()
{
#include "FortNativeGameplayTagsRegistration.inc"
}
