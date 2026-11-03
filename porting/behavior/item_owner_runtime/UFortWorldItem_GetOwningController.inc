// Original item owner/lifetime behavior; see HANDOFF.md.
// CONFIDENCE: HIGH
if (NativeOwningController.IsValid())
{
    return static_cast<AFortPlayerController*>(NativeOwningController.Get());
}
if (NativeGetOwnerInventory())
{
    AFortInventory* Inventory = NativeGetOwnerInventory();
    return Cast<AFortPlayerController>(Inventory->GetOwner());
}
return nullptr;
