// Behaviour snippet: UInventoryManagementLibrary::GetInventoryNetworkComponent (body only; spliced into the generated function).
// Original body address: 0x14200cc10. Offset evidence: Owner (RCX) null -> XOR EAX,EAX; otherwise MOV RAX,[RCX+0x2008] = AFortPlayerController.InventoryNetworkManagementComponent (Object, reflected at +0x2008).
// CONFIDENCE: MEDIUM
if (!Owner) { return nullptr; }
return reinterpret_cast<UFortControllerComponent_InventoryNetworkManagement*>(Owner->InventoryNetworkManagementComponent);
