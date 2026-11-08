// Original body address: 0x141aae6e0 (thunk 0x1427e8db0)
// Offset evidence: this+0x12b0 = TArray data pointer and this+0x12b8 = Num (unreflected: NativeRecoveredUint64_0x12b0 / NativeRecoveredInt32_0x12b8); element stride 0x80, key pointer at element+0x10, score int32 at element+0x50.
// AthenaPC->PlayerState is at +0x220 (AController::PlayerState); the IsA test against AFortPlayerStateAthena::StaticClass() is inlined and is modelled as Cast<>; the first element whose key equals that cast result returns true with its score.
// Entire 48-instruction body read against the port; MEDIUM because the inlined IsA lowering is inferred.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AFortPlayerStateAthena) > 0, "AFortPlayerStateAthena must be complete for the Cast test");
if (!AthenaPC)
{
	return false;
}
const uint8* Entry = reinterpret_cast<const uint8*>(NativeRecoveredUint64_0x12b0);
const uint8* const End = Entry + static_cast<int64>(NativeRecoveredInt32_0x12b8) * 0x80;
if (Entry == End)
{
	return false;
}
AFortPlayerStateAthena* ScorePlayerState = Cast<AFortPlayerStateAthena>(AthenaPC->PlayerState);
do
{
	const void* const EntryKey = *reinterpret_cast<const void* const*>(Entry + 0x10);
	if (EntryKey == static_cast<const void*>(ScorePlayerState))
	{
		OutScore = *reinterpret_cast<const int32*>(Entry + 0x50);
		return true;
	}
	Entry += 0x80;
} while (Entry != End);
return false;
