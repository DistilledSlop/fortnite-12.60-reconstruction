// dep AFortGameStateZone__RegisterGameplayModifier_Implementation original 0x141ea8c40
// CONFIDENCE: MEDIUM. Original 0x141ea8c40. Returns INDEX_NONE unless the authority registers a valid modifier.
FGameplayModifierHandle Handle = FGameplayModifierHandle(INDEX_NONE);
if (Role != ROLE_Authority || Modifier == nullptr)
{
	return Handle;
}
AddGameplayModifierEntry(ActiveGameplayModifiers, Handle, this, Modifier, Expiration);   // 0x141e8d570
bool bHasBoundSlot = false;
for (const FModifierDelegateSlot& Slot : ModifierDelegateList)   // +0x9c0 pairs, count +0x9c8
{
	if (Slot.Object != nullptr && Slot.Count != 0)
	{
		bHasBoundSlot = true;
		break;
	}
}
if (bHasBoundSlot)
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
}
return Handle;
