// CONFIDENCE: MEDIUM - the removal loop and key construction are fully read; the object-match helper 0x140f6d100 and the removal hook 0x1423a8bd0 are not ported.
// Original address: 0x142a48b90 (multicast exec thunk, ParentBuilding object). Body via virtual slot 0x11c0 -> 0x142395330.
// Body builds an object key for ParentBuilding (InternalIndex, serial), then sweeps ActiveWeakSpots.
// An entry is removed when its weak building pointer is invalid or matches the key.
// Null ParentBuilding gives key (-1, 0). Key helper: 0x142ee3b30. Sweep helper: 0x142370520.
FObjectKeyLike Key;
if (ParentBuilding == nullptr)
{
	Key.Index = -1;
	Key.Serial = 0;
}
else
{
	Key.Index = ParentBuilding->InternalIndex;
	Key.Serial = GetObjectSerialNumber_0x142ec00d0();
}
for (int32 i = 0; i < ActiveWeakSpots.Num(); ++i)
{
	FWeakSpotEntry& Entry = ActiveWeakSpots[i];
	bool bRemove = !Entry.Building.IsValid();
	if (!bRemove)
	{
		bRemove = MatchesObjectKey_0x140f6d100(Entry.Building.Get(), &Key);
	}
	if (bRemove)
	{
		OnWeakSpotRemoved_0x1423a8bd0(Entry);
		ActiveWeakSpots.RemoveAt(i);
		--i;
	}
}
