// CONFIDENCE: LOW. Original exec 0x142a5cc20 (RPC thunk, no parameters).
// Validate is virtual slot 0x27b8 -> Stub_ReturnTrue (0x1406801d0), so no validation runs (STUB).
// Implementation is virtual slot 0x27c0 -> 0x1419f3510, inlined below.
// Helpers: controller virtual slot 0x1bd0 (quest manager lookup), FUN_14244f820 (bus driver handler, see ServerThankBusDriver), FUN_141a596b0 (sets bit 1 of PlayerState+0xfdd, read in full).
if (PlayerState != nullptr && PlayerState->IsA(AFortPlayerStateAthena::StaticClass()) && !PlayerState->GetThankBusDriverFlag())
{
	UObject* BusDriverTarget = GetBusDriverTarget(2); // controller virtual slot 0x1bd0 (kind 2)
	if (BusDriverTarget != nullptr)
	{
		FGameplayTagContainer BusTags;
		BusTags.AddTag(GetThankBusDriverSettings()->Field_0x15f8); // FUN_141ed1800 + 0x15f8
		FBusDriverResult ResultA;
		FBusDriverResult ResultB;
		bool bOutFlag = false;
		ProcessBusDriverThank(BusDriverTarget, this, BusTags, ResultA, ResultB, bOutFlag, true); // FUN_14244f820
		if (bOutFlag)
		{
			PlayerState->SetBusDriverCheckFlag(true); // FUN_141a596b0: PlayerState+0xfdd bit 1
		}
	}
}
