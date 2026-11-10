// CONFIDENCE: LOW. Original exec 0x142a5cbd0 (RPC thunk, no parameters).
// Validate is virtual slot 0x27c8 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x27d0 -> 0x1419f3650, inlined below.
// Helpers: FUN_141a5b620 (sets bit 0 of PlayerState+0xfdd, read in full), controller virtual slot 0x1bd0 (0x1423850c0, reads the quest manager at controller+0x19e8, read in full), FUN_143e8b3b0 (FGameplayTagContainer::AddTag, read in part), FUN_14244f820 (bus driver handler, its 7-argument form is not fully recovered; see deps).
if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()))
{
	PlayerState->SetThankBusDriverFlag(true); // FUN_141a5b620: PlayerState+0xfdd bit 0
	UObject* BusDriverTarget = GetBusDriverTarget(2); // controller virtual slot 0x1bd0 (kind 2)
	if (BusDriverTarget != nullptr)
	{
		FGameplayTagContainer BusTags;
		BusTags.AddTag(GetThankBusDriverSettings()->Field_0x15f0); // FUN_141ed1800 + 0x15f0
		FBusDriverResult ResultA; // outputs of FUN_14244f820 (not written by the visible part of the callee)
		FBusDriverResult ResultB;
		bool bOutFlag = false;
		ProcessBusDriverThank(BusDriverTarget, this, BusTags, ResultA, ResultB, bOutFlag, true); // FUN_14244f820
	}
}
