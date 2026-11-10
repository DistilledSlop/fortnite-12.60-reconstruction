// CONFIDENCE: MEDIUM
// Original addresses: RPC thunk 0x1428ce4b0; _Validate virtual slot 0xb80 -> 0x1406801d0 (Stub_ReturnTrue);
// _Implementation virtual slot 0xb88 -> 0x1426818d0.
// Validate always returns true in this build, so the RPC_ValidateFailed branch is kept for fidelity but never taken.
if (!ServerPressTrigger_Validate())
{
	RPC_ValidateFailed(TEXT("ServerPressTrigger_Validate"));
	return;
}
// _Implementation (0x1426818d0): same shape as ServerPressSecondaryFire, using the OnPressTrigger event (CallDecoBlueprintEvent 0x1428cbee0).
CallDecoBlueprintEvent(FName_OnPressTrigger, GetDecoHelper());
