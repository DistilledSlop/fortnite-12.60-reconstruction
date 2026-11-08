// Original body 0x1420a4140 (thunk 0x1429c5e00) for AFortMinigame::SetStatForGroup.
// Offset evidence: [RCX+0xf0] is AActor::Role (GetLocalRole(), 3 = ROLE_Authority); [RCX+0x870] is CurrentState (reflected enum byte); RCX+0x810 is RoundStats (reflected struct) passed as both owner and entry array.
// ABI: RDX = InFilter (TSubclassOf), R8D = InValue; tail-jumps to OnRep_StatsChanged(this).
// CONFIDENCE: HIGH
if (GetLocalRole() == ROLE_Authority && static_cast<uint8>(CurrentState) != 0)
{
	AFortMinigame::NativeSetStatFilterEntryValue1420a40c0(&RoundStats, reinterpret_cast<uint8*>(&RoundStats), InFilter.Get(), InValue);
	OnRep_StatsChanged();
}
