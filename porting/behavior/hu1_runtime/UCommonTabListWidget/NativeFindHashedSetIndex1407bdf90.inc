// Helper 0x1407bdf90. Caller body 0x1407c5300 passes Set=this+0x2b8 in RCX and Key in R8; caller 0x1407f04d0 passes Key in R8 from an element pair.
// Evidence: early-out when int32 at +0x8 == int32 at +0x34 -> -1.
// Hash = NativeHashUInt32142ce11c0(low32(Key)) + high32(Key) (32-bit add, from [RSP+0x44] spill of R8 high dword).
// Bucket = sign-extended(Hash) & (int64)(int32 at +0x48 - 1). Bucket array: inline at +0x38, or pointer at +0x40 when non-null; bucket value int32, -1 = empty.
// Elements: base pointer at +0x0, stride 40 bytes. Element key qword at +0 (low dword == KeyLo, high dword == KeyHi). Chain next int32 at element +0x20, -1 terminates.
// Returns matching element index, or -1.
// CONFIDENCE: MEDIUM
int32 NativeFindHashedSetIndex1407bdf90(void* Set, uint64 Key)
{
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
		const uint8* Element = ElementBase + static_cast<int64>(ChainIndex) * 40;
		const uint64 ElementKey = *reinterpret_cast<const uint64*>(Element);
		if (static_cast<uint32>(ElementKey) == KeyLo && static_cast<uint32>(ElementKey >> 32) == KeyHi)
		{
			return ChainIndex;
		}
		ChainIndex = *reinterpret_cast<const int32*>(Element + 0x20);
	}
	return -1;
}
