// CONFIDENCE: MEDIUM. Thunk 0x142a7e8c0 reads Part (byte) first and ChosenCharacterPart (object) second; reflection lists them in the opposite order, which is why the impl decompile shows swapped names. Validate (0x14234a270) is Part < 6. Impl 0x14234a150 stores the part into the customization component's array (0x141db8ac0), reinitialises each changed slot via the helper dep ReinitializeCharacterPart (0x142346450), then refreshes charms (0x14232dfc0), raises FName DAT_1481b4998 with no params (0x142a767c0), and resets a counter on the customization component (0x141db5c70). Unnamed vcalls: 0x1250 (part array), 0x11f0 (per-slot refresh), 0x1270 (after reinit, dep only).
if (!(Part < 6))
{
	RPC_ValidateFailed(TEXT("ServerChoosePart_Validate"));
	return;
}
AFortPlayerState* PlayerState = UFortKismetLibrary::AttemptGetPlayerStateForPawn(this);
UObject* CustomizationComp = nullptr;
if (!PlayerState)
{
	if (!Hero.Get())
	{
		return;
	}
	CustomizationComp = HACK_CustomPRIComponent;
}
else
{
	CustomizationComp = PlayerState->NativeRecovered_0x5d8;
}
if (CustomizationComp)
{
	CustomizationComp->NativeRecovered_0x38[Part] = ChosenCharacterPart;   // FUN_141db8ac0
	bool bAnyChanged = false;
	for (uint32 Slot = 0; Slot < 6; ++Slot)
	{
		const TArray<UObject*>* CurrentParts = GetCharacterPartArray(0);   // vcall 0x1250
		if (CurrentParts && (*CurrentParts)[Slot] != PreviousCharacterParts[Slot] && CharacterPartSkeletalMeshComponents[Slot])
		{
			ReinitializeCharacterPart(Slot);   // FUN_142346450, dep file
			RefreshCharacterPartSlot();   // vcall 0x11f0
			bAnyChanged = true;
		}
	}
	if (bAnyChanged)
	{
		InitializeCharacterCharms();   // FUN_14232dfc0
		ProcessEventCharacterPartsChanged();   // FUN_142a767c0 (FName DAT_1481b4998, no params)
		PlayerState = UFortKismetLibrary::AttemptGetPlayerStateForPawn(this);
		UObject* RefreshComp = nullptr;
		if (!PlayerState)
		{
			if (!Hero.Get())
			{
				return;
			}
			RefreshComp = HACK_CustomPRIComponent;
		}
		else
		{
			RefreshComp = PlayerState->NativeRecovered_0x5d8;
		}
		if (RefreshComp)
		{
			RefreshComp->NativeRecovered_0x58 = 0;   // FUN_141db5c70 (uint16 reset)
		}
	}
}
