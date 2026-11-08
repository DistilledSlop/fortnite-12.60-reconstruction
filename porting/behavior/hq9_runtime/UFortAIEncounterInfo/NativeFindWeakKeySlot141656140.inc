// Helper 0x141656140, a hash lookup over a map struct. Callers: EncounterHasReservedSpawnRequestForEnemySpawner (0x141655090),
// SpawnAIPawnReservedForEnemySpawner (0x1416724a0), UnRegisterAISpawner (0x1416776c0). Callers pass RCX=[this+0xa38] (the map), RDX=out int32*, R8=weak-pointer key (8 bytes).
// Evidence: Num = int32 [Map+0x8]; early out to -1 when Num == int32 [Map+0x34].
// Bucket table = qword [Map+0x40] when non-null, else inline at Map+0x38; HashSize = int32 [Map+0x48]; bucket = (sext(hi32 key) ^ sext(lo32 key)) & (HashSize-1).
// Element data = qword [Map+0], stride 0x70, key qword at +0, next index int32 at +0x68 (-1 ends chain).
// Each element key is compared with UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(elementKey, key).
// Writes the found index (or -1) to *OutIndex and returns OutIndex (RAX=R15).
// CONFIDENCE: MEDIUM
if (*reinterpret_cast<const int32*>(MapBase + 0x8) == *reinterpret_cast<const int32*>(MapBase + 0x34))
{
	*OutIndex = -1;
	return OutIndex;
}
const int64 HighHash = static_cast<int64>(static_cast<int32>(static_cast<uint64>(LookupKey) >> 32));
const int64 LowHash = static_cast<int64>(static_cast<int32>(static_cast<uint32>(LookupKey)));
const uint8* BucketOwner = *reinterpret_cast<const uint8* const*>(MapBase + 0x40);
const uint8* BucketBase = BucketOwner != nullptr ? BucketOwner : MapBase + 0x38;
const int64 HashSizeMinusOne = static_cast<int64>(*reinterpret_cast<const int32*>(MapBase + 0x48)) - 1;
const int64 BucketIndex = (HighHash ^ LowHash) & HashSizeMinusOne;
int32 SlotIndex = reinterpret_cast<const int32*>(BucketBase)[BucketIndex];
if (SlotIndex == -1)
{
	*OutIndex = -1;
	return OutIndex;
}
const uint8* ElementBase = *reinterpret_cast<const uint8* const*>(MapBase);
while (SlotIndex != -1)
{
	const uint8* ElementAddress = ElementBase + static_cast<int64>(SlotIndex) * 0x70;
	const uint64 ElementKey = *reinterpret_cast<const uint64*>(ElementAddress);
	if (UCreativeQuestComponent::NativeWeakObjectPtrEquivalent1407327b0(reinterpret_cast<const uint8*>(&ElementKey), reinterpret_cast<const uint8*>(&LookupKey)))
	{
		*OutIndex = SlotIndex;
		return OutIndex;
	}
	SlotIndex = *reinterpret_cast<const int32*>(ElementAddress + 0x68);
}
*OutIndex = -1;
return OutIndex;
