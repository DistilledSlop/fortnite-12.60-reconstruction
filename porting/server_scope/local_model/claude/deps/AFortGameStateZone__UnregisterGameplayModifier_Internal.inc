// dep AFortGameStateZone__UnregisterGameplayModifier_Internal original 0x141eb5700
// CONFIDENCE: MEDIUM. Original 0x141eb5700. Only the authority removes entries; INDEX_NONE is ignored.
if (Role == ROLE_Authority && Handle != INDEX_NONE)
{
	for (int32 EntryIndex = 0; EntryIndex < ActiveGameplayModifiers.Entries.Num(); ++EntryIndex)
	{
		FActiveGameplayModifier& Entry = ActiveGameplayModifiers.Entries[EntryIndex];
		if (Entry.Handle == Handle)
		{
			DeactivateGameplayModifierEntry(Entry, this);   // 0x141eb2b40 (partially read, see its evidence)
			ActiveGameplayModifiers.Entries.RemoveAt(EntryIndex, 1, /*bAllowShrink*/ true);   // 0x141ea9700
			ResetGameplayModifierDirtyBits(ActiveGameplayModifiers);   // 0x14072a990
			++ActiveGameplayModifiers.DirtySerial;                      // List+0x54
			if (ActiveGameplayModifiers.DirtySerial == INDEX_NONE)
			{
				ActiveGameplayModifiers.DirtySerial = 0;
			}
			ActiveGameplayModifiers.LastDirtyIndex = INDEX_NONE;       // List+0xf8
			break;
		}
	}
	for (const FModifierDelegateSlot& Slot : ModifierDelegateList)
	{
		if (Slot.Count != 0 && Slot.Object != nullptr)
		{
			TArray<UObject*> Sources;
			for (const FActiveGameplayModifier& Entry : ActiveGameplayModifiers.Entries)
			{
				if (Entry.Object != nullptr)
				{
					Sources.Add(Entry.Object);
				}
			}
			ModifierDelegateList.SetAndCompact(Sources);   // 0x1406ba850
			return;
		}
	}
}
