// Original body address: 0x142371d10 (thunk 0x142a44a00)
// Offset evidence: [RCX+0x1738] = WorldInventory (AFortInventory*, reflected Object at 0x1738); byte [RCX+0x179f] is an unreflected gap in the own region -> NativeRecoveredBool_0x179f (object offset 0x179f).
// Callee 0x1420147f0 -> AFortInventory::HandleInventoryLocalUpdate() (declared in AFortInventory.h), receiver = WorldInventory.
// Body: JZ on WorldInventory returns with no writes; otherwise flag byte set to 1 then tail-jump to the callee.
// CONFIDENCE: MEDIUM
if (WorldInventory != nullptr)
{
	NativeRecoveredBool_0x179f = true;
	WorldInventory->HandleInventoryLocalUpdate();
}
