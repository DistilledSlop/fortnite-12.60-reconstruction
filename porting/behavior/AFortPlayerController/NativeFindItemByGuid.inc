// CONFIDENCE: high
// 0x142380fd0 receives the interface at +0x598. Its +0x11a0/8/10
// match reflected WorldInventory/OutpostInventory/ViewTargetInventory.
if (WorldInventory)
{
    if (UFortWorldItem* Item = WorldInventory->NativeFindItemByGuid(ItemGuid))
        return Item;
}
if (OutpostInventory)
{
    if (UFortWorldItem* Item = OutpostInventory->NativeFindItemByGuid(ItemGuid))
        return Item;
}
return ViewTargetInventory ? ViewTargetInventory->NativeFindItemByGuid(ItemGuid) : nullptr;
