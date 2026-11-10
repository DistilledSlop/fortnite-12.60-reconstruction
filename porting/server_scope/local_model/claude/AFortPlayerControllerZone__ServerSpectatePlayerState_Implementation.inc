// CONFIDENCE: MEDIUM
// Original address: 0x1423e98c0 (vtable slot 0x2160 of AFortPlayerControllerZone). Capture: captures/5bbd05c09d0944f6a121c60cb7c608c9/0x1423e98c0.c
// PlayerToSpectate is the object whose spectate target is requested. Its virtual slot 0x840 is an unnamed boolean query (LOW: name not recovered).
// FUN_1419b6d20 = GetFortPlayerPawnOf(PlayerToSpectate): returns the pawn stored at +0x278 when that object is an AFortPlayerPawn, else null.
// FUN_142a6f480 = CallBlueprintEvent(this, FName event, pawn, bool): ProcessEvent of FindFunctionChecked(event) with {pawn, bAllowStateChange}.
if (PlayerToSpectate != nullptr && !PlayerToSpectate->IsSpectateBlockedQuery())
{
	AFortPlayerPawn* SpectatePawn = GetFortPlayerPawnOf(PlayerToSpectate);
	if (SpectatePawn != nullptr)
	{
		CallBlueprintEvent(SpectateStateChangedEvent, SpectatePawn, bAllowStateChange);
	}
}
