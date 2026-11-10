// CONFIDENCE: HIGH
// Original: exec thunk 0x142a430a0. ItemDescription is a by-reference FFortItemEntry; the thunk forwards the address
// to the virtual at slot 0x1b20 (reflected name AddInventoryItemWithAnimation). The base body (0x142368610) only logs
// "AddInventoryItemWithAnimation was NOT called from server!" when the Display log check passes; subclasses override.
NativeAddInventoryItemWithAnimation(ItemDescription);
