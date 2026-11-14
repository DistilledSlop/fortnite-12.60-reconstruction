// CONFIDENCE: HIGH. Original 0x142698590, weapon virtual +0x820.
if (!WeaponData)
{
    return false;
}
UFortWorldItemDefinition* AmmoDefinition = WeaponData->GetAmmoWorldItemDefinitionNative();
if (!AmmoDefinition)
{
    return false;
}
UFortAmmoItemDefinition* Ammo = Cast<UFortAmmoItemDefinition>(AmmoDefinition);
return !Ammo || Ammo->bIsConsumed;
