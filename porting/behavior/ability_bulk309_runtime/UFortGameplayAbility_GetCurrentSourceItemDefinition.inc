// CONFIDENCE: high; captured native branches, fields and engine contracts.
if (!IsInstantiated())
{
    return nullptr;
}
UObject* Source = GetSourceObject(CurrentSpecHandle, CurrentActorInfo);
if (UFortWorldItemDefinition* ItemDefinition = Cast<UFortWorldItemDefinition>(Source))
{
    return ItemDefinition;
}
if (AFortWeapon* Weapon = Cast<AFortWeapon>(Source))
{
    UFortWeaponItemDefinition* Definition = Weapon->WeaponData;
    return Definition;
}
return nullptr;
