// Helper 0x14108f3b0, host AFortAthenaMutator_PlayerClassSettings (static). 91 instructions, no callees.
// SetStorage (this+0x3c0 at callers): +0x0 element data pointer (20-byte elements: int32 key0 @0, key1 @4, next-in-chain @0xc);
//   +0x8 and +0x34 int32 counters compared for an early not-found exit; +0x38 inline bucket array; +0x40 heap bucket pointer;
//   +0x48 int32 HashSize. Key is two int32 words. Hash is the asm's 32-bit mixing sequence, transcribed step by step.
// Output: element index on match, -1 when not found; returns the output pointer (asm RAX = R11).
// No reflected member covers +0x3c0 or these offsets (raw access per asm widths).
// Callers: AFortAthenaMutator_PlayerClassSettings::AddDataSourceForPlayerClass (0x141af3ea0), RemoveDataSource (0x141b0ad20).
// CONFIDENCE: HIGH
if (*reinterpret_cast<const int32*>(SetStorage + 0x8) == *reinterpret_cast<const int32*>(SetStorage + 0x34))
{
	*OutIndex = -1;
	return OutIndex;
}
const uint32 KeyA = static_cast<uint32>(Key[0]);
const uint32 KeyB = static_cast<uint32>(Key[1]);
uint32 HashH0 = 0x9e3779b9u;
uint32 HashH1 = KeyA;
uint32 HashH2 = KeyB;
uint32 HashT = KeyB >> 13;
HashH1 = (HashH1 - KeyB) ^ HashT;
HashH0 -= HashH1;
HashT = HashH1 << 8;
HashH0 -= KeyB;
HashH0 ^= HashT;
HashH2 -= HashH0;
HashT = HashH0 >> 13;
HashH2 -= HashH1;
HashH2 ^= HashT;
HashH1 -= HashH0;
HashH1 -= HashH2;
HashT = HashH2 >> 12;
HashH1 ^= HashT;
HashH0 -= HashH1;
HashT = HashH1;
HashH0 -= HashH2;
HashT = HashH1 << 16;
HashH0 ^= HashT;
HashH2 -= HashH0;
HashT = HashH0 >> 5;
HashH2 -= HashH1;
HashH2 ^= HashT;
HashH1 -= HashH0;
HashH1 -= HashH2;
HashT = HashH2 >> 3;
HashH1 ^= HashT;
HashH0 -= HashH1;
HashT = HashH1;
HashH0 -= HashH2;
HashT = HashH1 << 10;
HashH0 ^= HashT;
HashH2 -= HashH0;
HashH2 -= HashH1;
const uint64 HashHighShift = static_cast<uint64>(HashH0) >> 15;
const int64 BucketRaw = static_cast<int64>(static_cast<int32>(HashH2)) ^ static_cast<int64>(HashHighShift);
const int64 BucketMask = static_cast<int64>(*reinterpret_cast<const int32*>(SetStorage + 0x48)) - 1;
const int64 BucketIndex = BucketRaw & BucketMask;
uint8* HeapBuckets = *reinterpret_cast<uint8* const*>(SetStorage + 0x40);
uint8* BucketBase = (HeapBuckets != nullptr) ? HeapBuckets : (SetStorage + 0x38);
int32 ElementIndex = *reinterpret_cast<const int32*>(BucketBase + BucketIndex * 4);
uint8* Elements = *reinterpret_cast<uint8* const*>(SetStorage + 0x0);
if (ElementIndex != -1)
{
	do
	{
		const uint8* Element = Elements + static_cast<int64>(ElementIndex) * 20;
		if (*reinterpret_cast<const int32*>(Element) == Key[0] && *reinterpret_cast<const int32*>(Element + 4) == Key[1])
		{
			*OutIndex = ElementIndex;
			return OutIndex;
		}
		ElementIndex = *reinterpret_cast<const int32*>(Element + 0xc);
	} while (ElementIndex != -1);
}
*OutIndex = -1;
return OutIndex;
