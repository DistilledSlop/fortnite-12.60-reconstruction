// Helper 0x141695990 on UFortAIEncounterInfo. Callers: GetEncounterRifts (0x14165af60), NotifyRiftDied (0x1416663d0); both pass RCX=this and RDX=out array.
// Evidence: out array is zeroed (Data, Num) then filled by two loops. Loop 1 walks begin=qword [this+0x68], count=int32 [this+0x70], stride 0x20.
// Loop 2 walks begin=qword [this+0xb0], count=int32 [this+0xb8], stride 0x20. Each iteration appends the qword at element+0x10 (TArray Add, grow via 0x140690bb0).
// Returns the out array reference (RAX=RBX).
// CONFIDENCE: MEDIUM
OutPointers.Empty();
const uint8* FirstBegin = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(EncounterInfo) + 0x68);
const uint8* FirstEnd = FirstBegin + static_cast<int64>(*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(EncounterInfo) + 0x70)) * 0x20;
for (const uint8* FirstElement = FirstBegin; FirstElement != FirstEnd; FirstElement += 0x20)
{
	OutPointers.Add(*reinterpret_cast<UObject* const*>(FirstElement + 0x10));
}
const uint8* SecondBegin = *reinterpret_cast<const uint8* const*>(reinterpret_cast<const uint8*>(EncounterInfo) + 0xb0);
const uint8* SecondEnd = SecondBegin + static_cast<int64>(*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(EncounterInfo) + 0xb8)) * 0x20;
for (const uint8* SecondElement = SecondBegin; SecondElement != SecondEnd; SecondElement += 0x20)
{
	OutPointers.Add(*reinterpret_cast<UObject* const*>(SecondElement + 0x10));
}
return OutPointers;
