// CONFIDENCE: HIGH
// Original 0x14269ed80.
IFortInventoryOwnerInterface* Owner = NativeGetInventoryOwnerInterface14269c8f0();
if (!Owner || !WeaponData)
{
    const uint32 Wrapped = uint32(MAX_int32) - uint32(GetMagazineAmmoCount());
    int32 Result;
    FMemory::Memcpy(&Result, &Wrapped, sizeof(Result));
    return Result;
}
UFortWorldItemDefinition* AmmoDefinition = WeaponData->GetAmmoWorldItemDefinitionNative();
if (!NativeUsesAmmo820() || Owner->NativeHasInfiniteAmmo(WeaponData))
{
    return MAX_int32;
}
if (!AmmoDefinition)
{
    return 0;
}
if (AmmoDefinition->NativeAllowsMultipleStacks380())
{
    TArray<UFortItem*> Items;
    Owner->NativeFindAllItemsA8(AmmoDefinition, Items);
    uint32 Sum = 0;
    for (UFortItem* Item : Items)
    {
        if (Item)
        {
            Sum += uint32(Item->GetNumInStack());
        }
    }
    int32 Result;
    FMemory::Memcpy(&Result, &Sum, sizeof(Result));
    return Result;
}
UFortItem* Item = Owner->NativeFindItemA0(AmmoDefinition, false);
return Item ? Item->GetNumInStack() : 0;
