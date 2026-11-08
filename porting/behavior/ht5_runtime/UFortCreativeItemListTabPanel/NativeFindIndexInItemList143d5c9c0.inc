// Helper body for UFortCreativeItemListTabPanel static helper at 0x143d5c9c0 (returns int32 index of RDX in the list, or -1).
// Asm: R9 = qword [RCX+0x2e8] (data), RCX = MOVSXD dword [RCX+0x2f0] (count); linear scan comparing qword [R9+i*8] with RDX; returns (RAX-R9)>>3 or 0xffffffff. No calls.
// Offset evidence: TArray layout Data +0x2e8, Num +0x2f0, Max +0x2f4. Reflection names +0x2e8 as OnSourceItemCountChanged (FOnItemCountChangedDelegate),
// but the helper scans it as an 8-byte-stride pointer array, so explicit array reads follow the asm. Element type is treated as UObject*.
// CONFIDENCE: MEDIUM
UObject** ScanData = *reinterpret_cast<UObject***>(reinterpret_cast<uint8*>(ListPanel) + 0x2e8);
int32 ScanCount = *reinterpret_cast<const int32*>(reinterpret_cast<uint8*>(ListPanel) + 0x2f0);
for (int32 ScanIndex = 0; ScanIndex < ScanCount; ++ScanIndex)
{
	if (ScanData[ScanIndex] == Target)
	{
		return ScanIndex;
	}
}
return -1;
