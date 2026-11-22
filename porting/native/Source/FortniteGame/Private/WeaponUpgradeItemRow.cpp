#include "WeaponUpgradeItemRow.h"
#include "ABuildingItemWeaponUpgradeActor.h"
#include "AFortInventory.h"
#include "AFortPawn.h"
#include "AFortPlayerController.h"
#include "AFortWeapon.h"
#include "FortAthenaInventoryFlow.h"
#include "FortAthenaLootTransfer.h"
#include "UFortResourceItemDefinition.h"
#include "UFortWorldItem.h"
#include "UFortWeaponRangedItemDefinition.h"

namespace
{
	bool ResolveMaterialCost(EFortWeaponUpgradeCosts Selector, EFortWeaponUpgradeCosts FirstSelector,
		int32 FirstLevel, const FScalableFloat& Curve, int32& OutCost)
	{
		const int32 Level = static_cast<int32>(Selector) - static_cast<int32>(FirstSelector) + FirstLevel;
		if (Level < FirstLevel || Level > 4)
		{
			return false;
		}
		const float Value = Curve.GetValueAtLevel(Level);
		if (!FMath::IsFinite(Value) || Value < 0.0f || Value >= static_cast<float>(MAX_int32))
		{
			return false;
		}
		OutCost = FMath::TruncToInt(Value);
		return true;
	}
}

bool FWeaponUpgradeItemRow::TryResolveMaterialCosts(const ABuildingItemWeaponUpgradeActor& Bench, FIntVector& OutCosts) const
{
	FIntVector Costs = FIntVector::ZeroValue;
	// 0x141c07360 evaluates the selected material curve at tier 1–4 (vertical)
	// or 0–4 (horizontal) and truncates the result toward zero.
	bool bResolved = false;
	switch (Direction)
	{
	case EFortWeaponUpgradeDirection::Vertical:
		bResolved =
			ResolveMaterialCost(WoodCost, EFortWeaponUpgradeCosts::WoodUncommon, 1, Bench.WoodCostCurve, Costs.X) &&
			ResolveMaterialCost(MetalCost, EFortWeaponUpgradeCosts::MetalUncommon, 1, Bench.MetalCostCurve, Costs.Y) &&
			ResolveMaterialCost(BrickCost, EFortWeaponUpgradeCosts::BrickUncommon, 1, Bench.BrickCostCurve, Costs.Z);
		break;
	case EFortWeaponUpgradeDirection::Horizontal:
		bResolved =
			ResolveMaterialCost(WoodCost, EFortWeaponUpgradeCosts::HorizontalWoodCommon, 0, Bench.HorizontalWoodCostCurve, Costs.X) &&
			ResolveMaterialCost(MetalCost, EFortWeaponUpgradeCosts::HorizontalMetalCommon, 0, Bench.HorizontalMetalCostCurve, Costs.Y) &&
			ResolveMaterialCost(BrickCost, EFortWeaponUpgradeCosts::HorizontalBrickCommon, 0, Bench.HorizontalBrickCostCurve, Costs.Z);
		break;
	default:
		break;
	}
	if (bResolved)
	{
		OutCosts = Costs;
	}
	return bResolved;
}

const FWeaponUpgradeItemRow* FWeaponUpgradeItemRow::FindRecipe(const UDataTable* Table,
	const UFortWeaponItemDefinition* Weapon, EFortWeaponUpgradeDirection Direction)
{
	if (!Table || Table->GetRowStruct() != StaticStruct() || !Weapon ||
		(Direction != EFortWeaponUpgradeDirection::Vertical && Direction != EFortWeaponUpgradeDirection::Horizontal))
	{
		return nullptr;
	}

	const FWeaponUpgradeItemRow* Result = nullptr;
	for (const TPair<FName, uint8*>& Entry : Table->GetRowMap())
	{
		const FWeaponUpgradeItemRow* Row = reinterpret_cast<const FWeaponUpgradeItemRow*>(Entry.Value);
		if (Row && Row->CurrentWeaponDef == Weapon && Row->Direction == Direction)
		{
			if (!Row->UpgradedWeaponDef || Row->UpgradedWeaponDef == Weapon)
			{
				continue;
			}
			if (Result && (Result->UpgradedWeaponDef != Row->UpgradedWeaponDef ||
				Result->WoodCost != Row->WoodCost || Result->MetalCost != Row->MetalCost || Result->BrickCost != Row->BrickCost))
			{
				return nullptr;
			}
			Result = Row;
		}
	}
	return Result;
}

UFortWeaponItemDefinition* ABuildingItemWeaponUpgradeActor::GetUpgradedItemFromTable(
	UFortWeaponItemDefinition* ItemToUpgrade, EFortWeaponUpgradeDirection UpgradeDirection) const
{
	const FWeaponUpgradeItemRow* Recipe = FWeaponUpgradeItemRow::FindRecipe(UpgradeTable, ItemToUpgrade, UpgradeDirection);
	return Recipe ? Recipe->UpgradedWeaponDef : nullptr;
}

bool FortWeaponUpgrade::TryUpgradeEquippedWeapon(ABuildingItemWeaponUpgradeActor& Bench, AFortPlayerController& Controller,
	EFortWeaponUpgradeDirection Direction)
{
	AFortPawn* Pawn = Cast<AFortPawn>(Controller.GetPawn());
	AFortInventory* Inventory = Controller.WorldInventory;
	if (!FortAthenaInventory::IsEnabled() || !Bench.HasAuthority() || !Controller.HasAuthority() ||
		!Pawn || Pawn->IsDead() || Pawn->bIsDying || Pawn->bIsDBNO || !Pawn->CurrentWeapon ||
		!Inventory || !Inventory->HasAuthority())
	{
		return false;
	}

	const FGuid WeaponGuid = Pawn->CurrentWeapon->GetInventoryGUID();
	const UFortWorldItem* Source = Inventory->NativeFindItemByGuid(WeaponGuid);
	const UFortWeaponItemDefinition* Definition = Source ? Cast<UFortWeaponItemDefinition>(Source->ItemEntry.ItemDefinition) : nullptr;
	const FWeaponUpgradeItemRow* Recipe = FWeaponUpgradeItemRow::FindRecipe(Bench.UpgradeTable, Definition, Direction);
	FIntVector Costs;
	if (!Recipe || Source->ItemEntry.Count != 1 || Recipe->UpgradedWeaponDef->NativeGetMaxStackSize(nullptr) > 1 ||
		!Recipe->TryResolveMaterialCosts(Bench, Costs))
	{
		return false;
	}

	// Some cooked ranged definitions omit the stack cap. GiveItem clamps their capacity
	// to one, but still follows the single-stack rule when IsStackable reports true.
	if (Recipe->UpgradedWeaponDef->IsStackable() && !Recipe->UpgradedWeaponDef->bAllowMultipleStacks &&
		Inventory->NativeFindWorldItemByDefinition(Recipe->UpgradedWeaponDef, true))
	{
		return false;
	}

	const UFortResourceItemDefinition* Materials[] = {
		Cast<UFortResourceItemDefinition>(Bench.WoodItem), Cast<UFortResourceItemDefinition>(Bench.MetalItem),
		Cast<UFortResourceItemDefinition>(Bench.BrickItem)
	};
	if (!Materials[0] || !Materials[1] || !Materials[2] ||
		Materials[0] == Materials[1] || Materials[0] == Materials[2] || Materials[1] == Materials[2])
	{
		return false;
	}

	struct FDebit
	{
		FGuid Guid;
		int32 Count;
	};
	TArray<FDebit> Debits;
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(Materials); ++Index)
	{
		int32 Remaining = Costs[Index];
		for (const UFortWorldItem* Item : Inventory->Inventory.ItemInstances)
		{
			if (Remaining == 0)
			{
				break;
			}
			if (Item && Item->ItemEntry.ItemDefinition == Materials[Index] && Item->ItemEntry.Count > 0)
			{
				const int32 Count = FMath::Min(Remaining, Item->ItemEntry.Count);
				Debits.Add({Item->ItemEntry.ItemGuid, Count});
				Remaining -= Count;
			}
		}
		if (Remaining != 0)
		{
			return false;
		}
	}

	const int32 Slot = FortAthenaLootTransfer::FindReplacementPrimarySlot(&Controller, WeaponGuid, Recipe->UpgradedWeaponDef);
	if (Slot == INDEX_NONE)
	{
		return false;
	}

	// Defer source ability teardown so its callbacks cannot observe or interrupt the batch.
	for (const FDebit& Debit : Debits)
	{
		FortAthenaInventory::RemoveItem(&Controller, Debit.Guid, Debit.Count, true);
	}
	FortAthenaInventory::RemoveItem(&Controller, WeaponGuid, 1, true);
	UFortWorldItem* Replacement = FortAthenaInventory::GiveItem(&Controller, Recipe->UpgradedWeaponDef, 1, 1, 0, nullptr, Slot, true);
	if (Replacement)
	{
		FortAthenaInventory::ExecuteInventoryItem(&Controller, Replacement->NativeGetItemGuid());
	}
	return Replacement != nullptr;
}
