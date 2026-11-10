// CONFIDENCE: MEDIUM
// Original addresses: RPC thunk 0x1428ce460; _Validate virtual slot 0xb50 -> 0x1406801d0 (Stub_ReturnTrue);
// _Implementation virtual slot 0xb58 -> 0x14267f2c0.
// Validate always returns true in this build, so the RPC_ValidateFailed branch is kept for fidelity but never taken.
if (!ServerPressSecondaryFire_Validate())
{
	RPC_ValidateFailed(TEXT("ServerPressSecondaryFire_Validate"));
	return;
}
// _Implementation (0x14267f2c0): hand the deco helper object to the OnPressSecondaryFire blueprint event.
// Helper (0x14269c7f0) returns this->DecoHelper; CallDecoBlueprintEvent (0x1428cbea0) does FindFunctionChecked + ProcessEvent.
CallDecoBlueprintEvent(FName_OnPressSecondaryFire, GetDecoHelper());
