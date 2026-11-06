#include "FortGadgetAmmo.h"
#include "UFortGadgetItemDefinition.h"
#include "UFortGameplayAbility.h"
#include "UFortItemDefinition.h"
#include "UFortWorldItemDefinition.h"
#include "FortAssets.h"
#include "FortDelayedObjectLoader.h"
#include "SharedFortGameLog.h"

UFortWorldItemDefinition* FortGetGadgetAmmoDefinition(const UFortGadgetItemDefinition* Definition)
{
    // CONFIDENCE: HIGH; 0x142008520, soft-class helper 0x141680900.
    if (Definition == nullptr)
    {
        UE_LOG(LogFort, Error, TEXT("nullptr gadget in GetAmmoWorldItemDefinition!"));
        return nullptr;
    }
    if (!Definition->IsValidLowLevel())
    {
        UE_LOG(LogFort, Error, TEXT("Invalid gadget in GetAmmoWorldItemDefinition!"));
        return nullptr;
    }

    UClass* AbilityClass = nullptr;
    if (!Definition->GameplayAbility.IsNull())
    {
        AbilityClass = Definition->GameplayAbility.Get();
        if (!AbilityClass)
            AbilityClass = Cast<UClass>(FortLoadDelayedObject(Definition->GameplayAbility.ToSoftObjectPath(), true));
    }
    FFortAssets::Get().TrackAsset(
        AbilityClass && AbilityClass->IsChildOf(UFortGameplayAbility::StaticClass()) ? AbilityClass : nullptr);
    if (!AbilityClass || !AbilityClass->IsChildOf(UFortGameplayAbility::StaticClass()))
        return nullptr;
    if (!AbilityClass->IsValidLowLevel())
    {
        UE_LOG(LogFort, Error, TEXT("GetAmmoWorldItemDefinition GameplayAbility invalid on gadget %s"), *Definition->GetName());
        return nullptr;
    }
    UFortGameplayAbility* AbilityCDO = static_cast<UFortGameplayAbility*>(AbilityClass->GetDefaultObject());
    if (!AbilityCDO)
    {
        UE_LOG(LogFort, Error, TEXT("GetAmmoWorldItemDefinition Null ability CDO on gadget %s"), *Definition->GetName());
        return nullptr;
    }
    if (!AbilityCDO->IsValidLowLevel())
    {
        UE_LOG(LogFort, Error, TEXT("GetAmmoWorldItemDefinition Invalid ability CDO on gadget %s"), *Definition->GetName());
        return nullptr;
    }
    static const FString Context(TEXT("UFortGadgetItemDefinition::GetAmmoWorldItemDefinition"));
    for (const FFortAbilityCost& Cost : AbilityCDO->AbilityCosts)
    {
        if (Cost.ItemDefinition && Cost.ItemDefinition->NativeGetItemType() == EFortItemType::Ammo &&
            FMath::TruncToInt(Cost.CostValue.GetValueAtLevel(1.0f, &Context)) > 0)
            return Cast<UFortWorldItemDefinition>(Cost.ItemDefinition);
    }
    return nullptr;
}
