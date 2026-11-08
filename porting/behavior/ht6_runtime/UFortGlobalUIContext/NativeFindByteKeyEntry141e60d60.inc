// Helper 0x141e60d60 (host UFortGlobalUIContext), chained-hash lookup of a uint8 key. Asm: 141e60d60..141e60dd1.
// Caller 0x144ccce30 (GetInputTypeIcon wrapper) tail-jumps here with RCX = the map owner and DL = key.
// Offset evidence (no reflected member covers these, so raw access):
//   int32 [+0x19b0] == int32 [+0x19dc] -> return null
//   bucket mask = (int64)int32[+0x19f0] - 1, masked with the zero-extended key byte
//   bucket heads: inline int32 array at +0x19e0, replaced by heap array when qword [+0x19e8] != 0
//   bucket head == -1 -> return null
//   entry array qword [+0x19a8], 24-byte entries: key byte @+0, value qword @+8, next int32 @+0x10, -1 ends chain
// Every instruction maps to a statement below.
// CONFIDENCE: HIGH
const uint8* MapBytes = reinterpret_cast<const uint8*>(MapOwner);
const int32 SparseCountA = *reinterpret_cast<const int32*>(MapBytes + 0x19B0);
const int32 SparseCountB = *reinterpret_cast<const int32*>(MapBytes + 0x19DC);
if (SparseCountA == SparseCountB)
{
	return nullptr;
}
const int64 BucketMask = (static_cast<int64>(*reinterpret_cast<const int32*>(MapBytes + 0x19F0)) - 1) & static_cast<int64>(LookupKey);
const uint8* BucketBase = MapBytes + 0x19E0;
const uint8* HeapBucketBase = *reinterpret_cast<const uint8* const*>(MapBytes + 0x19E8);
if (HeapBucketBase != nullptr)
{
	BucketBase = HeapBucketBase;
}
int32 EntryIndex = *reinterpret_cast<const int32*>(BucketBase + BucketMask * 4);
if (EntryIndex == -1)
{
	return nullptr;
}
const uint8* EntryBase = *reinterpret_cast<const uint8* const*>(MapBytes + 0x19A8);
while (true)
{
	const uint8* ChainEntry = EntryBase + static_cast<int64>(EntryIndex) * 24;
	if (*ChainEntry == LookupKey)
	{
		return *reinterpret_cast<void* const*>(ChainEntry + 8);
	}
	EntryIndex = *reinterpret_cast<const int32*>(ChainEntry + 0x10);
	if (EntryIndex == -1)
	{
		return nullptr;
	}
}
