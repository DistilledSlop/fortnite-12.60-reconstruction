// dep AFortGameStateZone__DeactivateGameplayModifierEntry original 0x141eb2b40
// CONFIDENCE: LOW. Original 0x141eb2b40 (7775 bytes). Only the first half was read in full; the rest is the same sparse-array walk over the second listener set (+0x88/+0xb0/+0x98) and was not read.
// First pass: walk the sparse listener set at Entry+0x38 (bit-array of valid slots); for each valid slot whose weak object resolves (0x14070f100), call its vtable +0x5c0 (listener deactivation notification). Then reset the set (0x141da4330).
for (FModifierListenerSlot& Slot : Entry.ListenerSetA)
{
	if (Slot.ID != INDEX_NONE)
	{
		if (UObject* Listener = Slot.Listener.Get())
		{
			Listener->OnGameplayModifierDeactivated(Entry, Owner);   // vtable +0x5c0
		}
	}
}
Entry.ListenerSetA.Reset();   // 0x141da4330
// Second listener set (+0x88) and the remaining cleanup were not read in full.
