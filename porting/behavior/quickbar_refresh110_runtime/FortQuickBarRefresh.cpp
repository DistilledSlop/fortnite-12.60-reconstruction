// Native quickbar refresh and its private dependency helpers.
#include "FQuickBar.h"
#include "AFortPlayerController.h"
#include "AFortInventory.h"
#include "AFortWeapon.h"
#include "UFortInventoryOwnerInterface.h"
#include "UFortItem.h"
#include "UFortWorldItemDefinition.h"
#include "UFortAlterationItemDefinition.h"
#include "UFortWeaponItemDefinition.h"
#include "UFortGadgetItemDefinition.h"
#include "UFortAssetManager.h"
#include "UFortGlobals.h"
#include "UFortGameInstance.h"
#include "FFortAlterationSlotStatus.h"
#include "FortAssets.h"
#include "FortDelayedObjectLoader.h"
#include "SharedFortGameLog.h"
#include "HAL/IConsoleManager.h"

static int32 GUnloadQuickbarItemsWhenUnequippedStW = 0;
static int32 GUnloadQuickbarItemsWhenUnequippedBR = 0;
static FAutoConsoleVariableRef CVarUnloadQuickbarItemsWhenUnequippedStW(
    TEXT("Fort.UnloadQuickbarItemsWhenUnequippedStW"), GUnloadQuickbarItemsWhenUnequippedStW,
    TEXT("When set to 1, quickbar items will be unloaded when equipped in StW. Not a graceful way to handle unloading these assets, and is only intended for legacy behavior"), ECVF_Default);
static FAutoConsoleVariableRef CVarUnloadQuickbarItemsWhenUnequippedBR(
    TEXT("Fort.UnloadQuickbarItemsWhenUnequippedBR"), GUnloadQuickbarItemsWhenUnequippedBR,
    TEXT("When set to 1, quickbar items will be unloaded when equipped in BR. Not a graceful way to handle unloading these assets, and is only intended for legacy behavior"), ECVF_Default);

static TArray<UFortAlterationItemDefinition*> QuickBar_GetActiveAlterations(UFortItem* Item, int32 PretendLevel)
{
    if (PretendLevel < 0)
        PretendLevel = Item->GetLevel();
    TArray<FFortAlterationSlotStatus> Slots = Item->GetAlterationSlots();
    TArray<UFortAlterationItemDefinition*> Result;
    if (Slots.Num() > 0)
        Result.Reserve(Slots.Num());
    for (const FFortAlterationSlotStatus& Slot : Slots)
        if (Slot.MinRequiredLevel <= PretendLevel &&
            uint8(Slot.MinHostItemRarity) <= uint8(Item->GetRarity()) && Slot.Alteration)
            Result.Add(Slot.Alteration);
    return Result;
}

// 0x1420140e0 / 0x141ff80b0: soft resolve, load if necessary, track
// even a failed non-null-path lookup. Same contract as weapon alteration getters.
static UFortWeaponItemDefinition* QuickBar_GetGadgetWeapon(UFortGadgetItemDefinition* Gadget)
{
    auto& Soft = Gadget->WeaponItemDefinition;
    if (Soft.IsNull())
        return nullptr;
    UFortWeaponItemDefinition* Result = Soft.Get();
    if (!Result)
        Result = Cast<UFortWeaponItemDefinition>(FortLoadDelayedObject(Soft.ToSoftObjectPath(), true));
    FFortAssets::Get().TrackAsset(Result);
    return Result;
}

// 0x141c96700, with native bLoad=true. The load path tracks the result but
// contributes no element to the returned array, even when loading succeeds.
static TArray<UFortItemDefinition*> QuickBar_GetAdditionalGadgetItems(UFortGadgetItemDefinition* Gadget)
{
    TArray<UFortItemDefinition*> Result;
    for (const TSoftObjectPtr<UFortItemDefinition>& Soft : Gadget->AdditionalItemsToLoadWhenEquipped)
    {
        if (Soft.IsNull())
            continue;
        UFortItemDefinition* Resolved = Soft.Get();
        UFortItemDefinition* Tracked = Resolved;
        if (!Resolved)
            Tracked = Cast<UFortItemDefinition>(FortLoadDelayedObject(Soft.ToSoftObjectPath(), true));
        FFortAssets::Get().TrackAsset(Tracked);
        if (Resolved)
            Result.Add(Resolved);
    }
    return Result;
}

// 0x142050360. The native bundle FName is Equipped.
static void QuickBar_ProcessDefinitions_142050360(
    const TArray<UFortItemDefinition*>& Definitions, bool bAdd)
{
    if (Definitions.Num() <= 0)
        return;

    UFortAssetManager& Manager = UFortAssetManager::NativeGetFortAssetManager();
    TArray<FPrimaryAssetId> AssetIds;
    for (UFortItemDefinition* Definition : Definitions)
    {
        if (!Definition)
            continue;
        AssetIds.AddUnique(Definition->GetPrimaryAssetId());
        TArray<UFortWeaponItemDefinition*> Weapons;
        if (UFortGadgetItemDefinition* Gadget = Cast<UFortGadgetItemDefinition>(Definition))
        {
            if (UFortWeaponItemDefinition* Weapon = QuickBar_GetGadgetWeapon(Gadget))
            {
                AssetIds.AddUnique(Weapon->GetPrimaryAssetId());
                Weapons.Add(Weapon);
            }
            for (UFortItemDefinition* Additional : QuickBar_GetAdditionalGadgetItems(Gadget))
            {
                if (Additional)
                {
                    AssetIds.AddUnique(Additional->GetPrimaryAssetId());
                    if (UFortWeaponItemDefinition* Weapon = Cast<UFortWeaponItemDefinition>(Additional))
                        Weapons.Add(Weapon);
                }
            }
        }
        else if (UFortWeaponItemDefinition* Weapon = Cast<UFortWeaponItemDefinition>(Definition))
            Weapons.Add(Weapon);

        for (UFortWeaponItemDefinition* Weapon : Weapons)
        {
            if (UFortAlterationItemDefinition* Alteration = Weapon->GetBaseAlteration())
                AssetIds.AddUnique(Alteration->GetPrimaryAssetId());
            if (UFortAlterationItemDefinition* Alteration = Weapon->GetBaseCosmeticAlteration())
                AssetIds.AddUnique(Alteration->GetPrimaryAssetId());
        }
    }
    TArray<FName> Bundle{FName(TEXT("Equipped"))};
    TArray<FName> Empty;
    Manager.ChangeBundleStateForPrimaryAssets(AssetIds, bAdd ? Bundle : Empty,
        bAdd ? Empty : Bundle, false, FStreamableDelegate(), 0);
}

void FQuickBar::RefreshEquippedItemDefinitions(AFortPlayerController* Owner)
{
    if (!Owner)
        return;

    EquippedItemDefinitions.Remove(nullptr);
    TSet<UFortItemDefinition*> NewDefinitions;
    for (int32 Index = 0; Index < Slots.Num(); ++Index)
    {
        const FQuickBarSlot& Slot = Slots[Index];
        const int32 Count = Slot.Items.Num();
        if (Count == 0)
            continue;
        // Native has a fallback for a corrupt negative array count.
        const FGuid Guid = Count > 0 ? Slot.Items[0] :
            (Slot.bIsOccupied ? Slot.UsedByItemGuid : FGuid());
        UFortItem* Item = Owner->NativeFindItemByGuid(Guid);
        UFortWorldItemDefinition* Definition = Item ?
            Cast<UFortWorldItemDefinition>(Item->GetItemDefinitionBP()) : nullptr;
        if (!Definition)
        {
            if (Guid.IsValid())
                UE_LOG(LogFort, Warning, TEXT("FQuickBar::RefreshEquippedItemDefinitions could not find Item with GUID %s"), *Guid.ToString());
            continue;
        }
        NewDefinitions.Add(Definition);
        // 0x14202b8d0: negative pretend level calls GetLevel; fetch a snapshot
        // of alteration slots, then for every level-qualified slot call GetRarity
        // even when Alteration is null; accept when rarity qualifies and non-null.
        for (UFortAlterationItemDefinition* Alteration : QuickBar_GetActiveAlterations(Item, -1))
            if (Alteration)
                NewDefinitions.Add(Alteration);
    }

    // Interface getters are virtual on Owner's subobject at +0x598.
    if (AFortWeapon* Weapon = static_cast<IFortInventoryOwnerInterface*>(Owner)->NativeQuickBarHeldWeapon())
        if (UFortWeaponItemDefinition* WeaponDefinition = Weapon->GetWeaponData())
            if (AFortInventory* Inventory = static_cast<IFortInventoryOwnerInterface*>(Owner)->NativeQuickBarInventory())
                if (Inventory->NativeFindWorldItemByDefinition(WeaponDefinition, false))
                    NewDefinitions.Add(WeaponDefinition);

    // Native builds a temporary difference set, then materializes it as an array.
    TArray<UFortItemDefinition*> FirstDifference = NewDefinitions.Difference(EquippedItemDefinitions).Array();
    QuickBar_ProcessDefinitions_142050360(FirstDifference, true);

    // Every mode query is preceded by the UFortGlobals singleton access and is
    // independently re-evaluated on the second branch.
    if ((GUnloadQuickbarItemsWhenUnequippedStW && (UFortGlobals::NativeGetGlobals(), UFortGameInstance::NativeGetGameMode141ed4e30(Owner)) == 0) ||
        (GUnloadQuickbarItemsWhenUnequippedBR && (UFortGlobals::NativeGetGlobals(), UFortGameInstance::NativeGetGameMode141ed4e30(Owner)) == 1))
    {
        TArray<UFortItemDefinition*> SecondDifference = EquippedItemDefinitions.Difference(NewDefinitions).Array();
        QuickBar_ProcessDefinitions_142050360(SecondDifference, false);
    }
    EquippedItemDefinitions = NewDefinitions;
}
