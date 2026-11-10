// CONFIDENCE: MEDIUM. Impl at 0x1423abed0 is fully read: it forwards to the inventory object.
// Inventory is at this+0x598. The forwarded call is virtual slot 0xb8 on that object, not read.
WorldInventory->RemoveInventoryItem(ItemGuid, Count, bForceRemoveFromQuickBars, bForceRemoval, bForcePersistWhenEmpty);
