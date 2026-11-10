// CONFIDENCE: HIGH
// Original: exec thunk 0x142a42fd0. The thunk copies the by-value FFortItemEntry (0x142023c10 default ctor,
// 0x142025400 assignment, wrapped by 0x1420237c0) and passes that copy to the virtual at slot 0x1b28.
// Base body of that slot (0x142368600) is empty: it only destroys its by-value argument, so the base does nothing.
const FFortItemEntry ItemCopy = ItemDescription;
NativeAddInventoryItemWithAlterationBasedOnLootLevel(ItemCopy); // vslot 0x1b28, unnamed in headers
