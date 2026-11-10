// CONFIDENCE: MEDIUM
// Original addresses: RPC thunk 0x1428ce500; _Validate virtual slot 0xb38 -> 0x1406801d0 (Stub_ReturnTrue);
// _Implementation virtual slot 0xb40 -> 0x1426801c0.
// Validate always returns true in this build, so the RPC_ValidateFailed branch is kept for fidelity but never taken.
if (!ServerReleaseSecondaryFire_Validate())
{
	RPC_ValidateFailed(TEXT("ServerReleaseSecondaryFire_Validate"));
	return;
}
// _Implementation (0x1426801c0): same shape, using the OnReleaseSecondaryFire event (CallDecoBlueprintEvent 0x1428cbf20).
CallDecoBlueprintEvent(FName_OnReleaseSecondaryFire, GetDecoHelper());
