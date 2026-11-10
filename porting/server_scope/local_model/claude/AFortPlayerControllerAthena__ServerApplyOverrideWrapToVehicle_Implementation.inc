// CONFIDENCE: MEDIUM
// Original target 0x1419f1040.
if (Pawn == nullptr)
{
	return;
}
// AFortPlayerPawn::StaticClass() check on the controller's pawn (FUN_142a75f10 is the cached static class).
if (!Pawn->IsA(AFortPlayerPawn::StaticClass()))
{
	return;
}
// vslot 0x1088 on the pawn: NativeGetVehicleInterface163 (existing port) returns the IFortVehicleInterface or null.
auto* VehicleInterface = Pawn->NativeGetVehicleInterface163();
if (VehicleInterface != nullptr)
{
	VehicleInterface->NativeApplyOverrideWrap(ItemWrap);	// vslot 0x5d0 on the interface (name not resolved)
}
