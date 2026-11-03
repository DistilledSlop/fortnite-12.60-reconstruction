#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

// One owner for the original 0x141ed1800 local-static singleton.
// Original offsets identify recovered fields; this type is not an ABI overlay.
class FORTNITEGAME_API FFortNativeGameplayTagsRuntime final
{
public:
    static FFortNativeGameplayTagsRuntime& Get();

    const FGameplayTag& GetTagAtOriginalOffset(uint32 Offset) const;
    const FGameplayTag& AthenaCreative() const { return GetTagAtOriginalOffset(0x1748); }
    const FGameplayTag& AthenaCreativeMM() const { return GetTagAtOriginalOffset(0x1750); }
    const FGameplayTag& AthenaCreativeLTM() const { return GetTagAtOriginalOffset(0x1758); }
    const FGameplayTag& AthenaCreativeUtilitySpawning() const { return GetTagAtOriginalOffset(0x1760); }
    const FGameplayTag& AthenaPlayground() const { return GetTagAtOriginalOffset(0x1768); }
    const FGameplayTag& AthenaBattleLab() const { return GetTagAtOriginalOffset(0x1770); }
    const FGameplayTag& GameplayActionPlayerChangeEquipment() const { return GetTagAtOriginalOffset(0x6c0); }
    const FGameplayTag& EventWeaponsWeaponSwitch() const { return GetTagAtOriginalOffset(0x160); }
    const FGameplayTag& AthenaHideForCapture() const { return GetTagAtOriginalOffset(0x24c0); }

    const TMap<uint8, FGameplayTag>& MovementModeTags() const { return MovementModeTags880; }
    const TMap<uint8, FGameplayTag>& CustomMovementModeTags() const { return CustomMovementModeTags8d0; }
    const FGameplayTagContainer& ContainerAt1b10() const { return Tags1b10; }
    const FGameplayTagContainer& ContainerAt1b30() const { return Tags1b30; }
    const TMap<FGameplayTag, int32>& TagValuesAt2058() const { return TagValues2058; }
    const TMap<FGameplayTag, int32>& TagValuesAt20a8() const { return TagValues20a8; }
    const TSet<FGameplayTag>& GameplayCueTags() const { return GameplayCueTags20f8; }

private:
    FFortNativeGameplayTagsRuntime();
    ~FFortNativeGameplayTagsRuntime() = default;
    FFortNativeGameplayTagsRuntime(const FFortNativeGameplayTagsRuntime&) = delete;
    FFortNativeGameplayTagsRuntime& operator=(const FFortNativeGameplayTagsRuntime&) = delete;

    void Register(uint32 Offset, const ANSICHAR* Name);
    void RegisterMovement(uint32 Offset, const ANSICHAR* Name, uint8 Value, bool bCustom);

    // All tag fields, including the three unregistered invalid fields, start zero.
    FGameplayTag TagsByOriginalOffset[0x2568 / 8];
    // Declaration order retains the original reverse destruction order.
    TMap<uint8, FGameplayTag> MovementModeTags880;
    TMap<uint8, FGameplayTag> CustomMovementModeTags8d0;
    FGameplayTagContainer Tags1b10;
    FGameplayTagContainer Tags1b30;
    TMap<FGameplayTag, int32> TagValues2058;
    TMap<FGameplayTag, int32> TagValues20a8;
    TSet<FGameplayTag> GameplayCueTags20f8;
};
