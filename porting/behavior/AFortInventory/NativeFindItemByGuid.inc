// CONFIDENCE: high
// 0x14200d0e0 receives AFortInventory's interface at +0x218.
// Interface +0x178 is Inventory (+0x228).ItemInstances (+0x168).
for (UFortWorldItem* Item : Inventory.ItemInstances)
{
    if (Item->NativeGetItemGuid() == ItemGuid)
    {
        return Item;
    }
}
return nullptr;
