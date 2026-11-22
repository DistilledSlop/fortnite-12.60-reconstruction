#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EFortWeaponUpgradeDirection.h"
#include "WeaponUpgradeItemRow.generated.h"

class ABuildingItemWeaponUpgradeActor;
class UFortWeaponItemDefinition;
class UFortWeaponRangedItemDefinition;

// Serialized by AthenaWumbaData. Reflection blocks 0x14790a5c8 and 0x1479092c0.
UENUM()
enum class EFortWeaponUpgradeCosts : uint8
{
	NotSet = 0,
	WoodUncommon = 1,
	WoodRare = 2,
	WoodVeryRare = 3,
	WoodSuperRare = 4,
	MetalUncommon = 5,
	MetalRare = 6,
	MetalVeryRare = 7,
	MetalSuperRare = 8,
	BrickUncommon = 9,
	BrickRare = 10,
	BrickVeryRare = 11,
	BrickSuperRare = 12,
	HorizontalWoodCommon = 13,
	HorizontalWoodUncommon = 14,
	HorizontalWoodRare = 15,
	HorizontalWoodVeryRare = 16,
	HorizontalWoodSuperRare = 17,
	HorizontalMetalCommon = 18,
	HorizontalMetalUncommon = 19,
	HorizontalMetalRare = 20,
	HorizontalMetalVeryRare = 21,
	HorizontalMetalSuperRare = 22,
	HorizontalBrickCommon = 23,
	HorizontalBrickUncommon = 24,
	HorizontalBrickRare = 25,
	HorizontalBrickVeryRare = 26,
	HorizontalBrickSuperRare = 27,
};

USTRUCT()
struct FWeaponUpgradeItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY()
	UFortWeaponRangedItemDefinition* CurrentWeaponDef = nullptr;

	UPROPERTY()
	UFortWeaponRangedItemDefinition* UpgradedWeaponDef = nullptr;

	UPROPERTY()
	EFortWeaponUpgradeCosts WoodCost = EFortWeaponUpgradeCosts::NotSet;

	UPROPERTY()
	EFortWeaponUpgradeCosts MetalCost = EFortWeaponUpgradeCosts::NotSet;

	UPROPERTY()
	EFortWeaponUpgradeCosts BrickCost = EFortWeaponUpgradeCosts::NotSet;

	UPROPERTY()
	EFortWeaponUpgradeDirection Direction = EFortWeaponUpgradeDirection::NotSet;

	// Costs are returned in wood, metal, brick order.
	bool TryResolveMaterialCosts(const ABuildingItemWeaponUpgradeActor& Bench, FIntVector& OutCosts) const;

	static const FWeaponUpgradeItemRow* FindRecipe(const UDataTable* Table,
		const UFortWeaponItemDefinition* Weapon, EFortWeaponUpgradeDirection Direction);
};
