// CONFIDENCE: HIGH
// Original: exec thunk 0x142a43600; forwarder 0x14236b810. The forwarder copies the FGuid by value and calls vtable slot
// 0xb0 on the interface at +0x598, which is NativeFindItemByGuid (the same slot used by NativeFindItemByGuid.inc).
return InventoryInterface->NativeFindItemByGuid(ItemGuid);
