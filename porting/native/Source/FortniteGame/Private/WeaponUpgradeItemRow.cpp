#include "WeaponUpgradeItemRow.h"
#include "UFortWeaponItemDefinition.h"

const FWeaponUpgradeItemRow* FWeaponUpgradeItemRow::FindRecipe(const UDataTable* Table,
	const UFortWeaponItemDefinition* Weapon, EFortWeaponUpgradeDirection Direction)
{
	if (!Table || Table->GetRowStruct() != StaticStruct() || !Weapon || Direction == EFortWeaponUpgradeDirection::NotSet)
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
