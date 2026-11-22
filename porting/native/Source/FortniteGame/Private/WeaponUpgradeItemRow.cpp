#include "WeaponUpgradeItemRow.h"
#include "ABuildingItemWeaponUpgradeActor.h"
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
