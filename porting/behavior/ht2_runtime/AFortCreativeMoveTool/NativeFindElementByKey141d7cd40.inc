// Helper 0x141d7cd40, host AFortCreativeMoveTool (static). 22 instructions, no callees.
// Container layout from asm (raw, no reflected member on the owner): element pointer array at +0x30, int32 Num at +0x38.
// SearchKey is a uint64: low dword compared with element +0x110, high dword compared with element +0x114.
// Returns the first matching element as UObject*, else nullptr.
// Callers: AFortCreativeMoveTool::HandleDropToFloorPressed (0x142673fe0), HandleHitTraceRuleHeldDownLongEnough (0x142674d80).
// CONFIDENCE: HIGH
uint8* ContainerBase = reinterpret_cast<uint8*>(ContainerObject);
uint8** ElementData = *reinterpret_cast<uint8***>(ContainerBase + 0x30);
const int32 ElementCount = *reinterpret_cast<const int32*>(ContainerBase + 0x38);
const uint32 KeyLow = static_cast<uint32>(SearchKey);
const uint32 KeyHigh = static_cast<uint32>(SearchKey >> 32);
for (int32 ElementIndex = 0; ElementIndex < ElementCount; ++ElementIndex)
{
	uint8* Candidate = ElementData[ElementIndex];
	const bool bHighMatches = *reinterpret_cast<const uint32*>(Candidate + 0x114) == KeyHigh;
	const bool bLowMatches = *reinterpret_cast<const uint32*>(Candidate + 0x110) == KeyLow;
	if (bHighMatches && bLowMatches)
	{
		return reinterpret_cast<UObject*>(Candidate);
	}
}
return nullptr;
