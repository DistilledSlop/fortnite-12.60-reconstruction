// Helper 0x140709840. Caller body 0x142513680 passes Set=this+0x3d0 and Key in R8; caller 0x1441399f0 passes Set=(ptr)+0x80 and Key in R8.
// Evidence: same hashed-set lookup as 0x1407bdf90 (early-out on int32 +0x8 == int32 +0x34; hash = Hash32(low32(Key)) + high32(Key) via NativeHashUInt32142ce11c0;
// bucket = sign-extended hash & (int32 at +0x48 - 1); bucket array inline +0x38 or pointer at +0x40; element base at +0x0).
// Difference: element stride is 20 bytes (LEA RDX,[RCX+RCX*4]; [R11+RDX*4]) and chain next int32 is at element +0xc.
// Returns matching element index, or -1.
// CONFIDENCE: MEDIUM
const uint8* SetBytes = static_cast<const uint8*>(Set);
const int32 CountA = *reinterpret_cast<const int32*>(SetBytes + 0x8);
const int32 CountB = *reinterpret_cast<const int32*>(SetBytes + 0x34);
if (CountA == CountB)
{
	return -1;
}

const uint32 KeyLo = static_cast<uint32>(Key);
const uint32 KeyHi = static_cast<uint32>(Key >> 32);
const uint32 HashValue = UFortGameplayAbility::NativeHashUInt32142ce11c0(KeyLo) + KeyHi;

const int32 HashCount = *reinterpret_cast<const int32*>(SetBytes + 0x48);
const int64 BucketIndex = static_cast<int64>(static_cast<int32>(HashValue)) & static_cast<int64>(HashCount - 1);

const void* OutOfLinePtr = *reinterpret_cast<const void* const*>(SetBytes + 0x40);
const uint8* BucketBase = OutOfLinePtr ? static_cast<const uint8*>(OutOfLinePtr) : (SetBytes + 0x38);
int32 ChainIndex = *reinterpret_cast<const int32*>(BucketBase + BucketIndex * 4);
if (ChainIndex == -1)
{
	return -1;
}

const uint8* ElementBase = *reinterpret_cast<const uint8* const*>(SetBytes);
while (ChainIndex != -1)
{
	const uint8* Element = ElementBase + static_cast<int64>(ChainIndex) * 20;
	const uint64 ElementKey = *reinterpret_cast<const uint64*>(Element);
	if (static_cast<uint32>(ElementKey) == KeyLo && static_cast<uint32>(ElementKey >> 32) == KeyHi)
	{
		return ChainIndex;
	}
	ChainIndex = *reinterpret_cast<const int32*>(Element + 0xc);
}
return -1;
