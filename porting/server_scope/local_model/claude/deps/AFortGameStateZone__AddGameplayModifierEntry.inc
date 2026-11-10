// dep AFortGameStateZone__AddGameplayModifierEntry original 0x141e8d570
// CONFIDENCE: LOW. Original 0x141e8d570. List is ActiveGameplayModifiers (this+0xa28). Entries are 0xd8-byte records at List+0x108, count List+0x110, capacity List+0x114.
OutHandle = FGameplayModifierHandle(INDEX_NONE);
if (Modifier != nullptr)
{
	const int32 Serial = List.SerialNumber;          // List+0x128
	List.SerialNumber = Serial + 1;
	const int32 NewIndex = List.Entries.Num();        // List+0x110
	List.Entries.AddDefaulted();                      // grows via 0x14072c520 when capacity is exceeded
	FActiveGameplayModifier& Entry = List.Entries[NewIndex];   // entry init 0x141e8b2a0 (defaults), 0x141e99170 / 0x1408a5b50 (container resets)
	Entry.Object = Modifier;                          // set via 0x141ea1410
	Entry.Owner = Owner;
	Entry.Expiration = Expiration;
	OutHandle = Entry.Handle;                         // piVar3[6] (+0x18)
	if (Entry.ID == INDEX_NONE)
	{
		++List.HandleCounter;                         // List+0x50
		Entry.ID = List.HandleCounter;
		if (List.HandleCounter == INDEX_NONE)
		{
			List.HandleCounter = 0;
		}
	}
	++Entry.RefCount;                                 // entry+4
	++List.DirtySerial;                               // List+0x54, wraps to 0 at -1
	if (List.DirtySerial == INDEX_NONE)
	{
		List.DirtySerial = 0;
	}
	List.LastDirtyIndex = INDEX_NONE;                 // List+0xf8
}
