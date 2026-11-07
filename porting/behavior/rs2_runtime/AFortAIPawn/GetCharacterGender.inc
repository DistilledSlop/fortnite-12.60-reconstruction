// Original body address: 0x1422c8060 (thunk 0x1427a47e0)
// Offset evidence: AppearanceOverrideEntries TArray at +0x1ad0 (Data), Num at +0x1ad8; AppearanceOverrideEntryIndex (Int) at +0x1ae0.
// Element stride 0x60 with a flag byte at element +0x8 (raw element read). Flag != 0 gives 2, else 1; out of range gives 0.
// CONFIDENCE: HIGH
uint8 Gender = 0;
if (AppearanceOverrideEntries.IsValidIndex(AppearanceOverrideEntryIndex))
{
	const uint8* ElementBase = reinterpret_cast<const uint8*>(AppearanceOverrideEntries.GetData());
	const uint8 Flag = ElementBase[static_cast<int64>(AppearanceOverrideEntryIndex) * 0x60 + 0x8];
	Gender = (Flag != 0) ? 2 : 1;
}
return static_cast<TEnumAsByte<EFortCustomGender::Type>>(Gender);
