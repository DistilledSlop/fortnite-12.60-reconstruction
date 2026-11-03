// CONFIDENCE: HIGH
UFortWeaponItemDefinition* Weapon = WeaponDefinition;
OutInfiniteAmmo = ItemsWithInfiniteAmmo.Contains(static_cast<UFortWorldItemDefinition*>(Weapon));
return 1;
