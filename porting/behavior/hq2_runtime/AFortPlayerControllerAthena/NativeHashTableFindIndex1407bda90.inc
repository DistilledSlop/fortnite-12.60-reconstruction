// Helper 0x1407bda90: hash-bucket lookup of a uint64 key in a table struct (out index via reference, RAX unused by callers).
// Offsets: Entries pointer +0x0 (24-byte entries: Key uint64 +0x0, Next int32 +0x10); inline bucket array +0x38;
// external bucket pointer +0x40 (used when non-null); bucket count int32 +0x48; early-out when int32 +0x8 == int32 +0x34.
// Hash: 32-bit Jenkins-style mix of (Key>>4) with constant 0x9e3779b9, final index = (sign-extended mix ^ (mix0>>15)) & (count-1).
// Callers (ABattleMapGroundMoveNode, ABuildingFoundation) read the index from their out memory, not RAX.
uint8* TableBase = static_cast<uint8*>(Table);
const int32 TableStamp = *reinterpret_cast<const int32*>(TableBase + 0x8);
const int32 TableStampRef = *reinterpret_cast<const int32*>(TableBase + 0x34);
if (TableStamp == TableStampRef)
{
    OutIndex = -1;
    return;
}
const uint32 HashA = static_cast<uint32>(Key >> 4);
uint32 HashX = 0x9e3779b9u - HashA;
HashX ^= (HashA << 8);
uint32 HashT = HashX >> 13;
uint32 HashY = HashX + HashA;
uint32 HashZ = HashA - HashX;
HashY = 0u - HashY;
HashY ^= HashT;
HashZ = HashZ - HashY;
HashT = HashY >> 12;
HashZ ^= HashT;
HashX = HashX - HashZ;
HashT = HashZ;
HashX = HashX - HashY;
HashT = HashZ << 16;
HashX ^= HashT;
HashY = HashY - HashX;
HashT = HashX >> 5;
HashY = HashY - HashZ;
HashY ^= HashT;
HashZ = HashZ - HashX;
HashZ = HashZ - HashY;
HashT = HashY >> 3;
HashZ ^= HashT;
HashX = HashX - HashZ;
HashT = HashZ;
HashX = HashX - HashY;
HashT = HashZ << 10;
HashX ^= HashT;
HashY = HashY - HashX;
const uint64 HashT64 = static_cast<uint64>(HashX) >> 15;
HashY = HashY - HashZ;
const uint64 BucketCountU = static_cast<uint64>(static_cast<int64>(*reinterpret_cast<const int32*>(TableBase + 0x48)));
const uint64 HashIndex = (static_cast<uint64>(static_cast<int64>(static_cast<int32>(HashY))) ^ HashT64) & (BucketCountU - 1);
const uint8* BucketPtrRaw = *reinterpret_cast<uint8* const*>(TableBase + 0x40);
const int32* Buckets = (BucketPtrRaw != nullptr) ? reinterpret_cast<const int32*>(BucketPtrRaw) : reinterpret_cast<const int32*>(TableBase + 0x38);
int32 EntryIndex = Buckets[HashIndex];
const uint8* Entries = *reinterpret_cast<const uint8* const*>(TableBase);
while (EntryIndex != -1)
{
    const uint8* Entry = Entries + static_cast<int64>(EntryIndex) * 24;
    if (*reinterpret_cast<const uint64*>(Entry) == Key)
    {
        OutIndex = EntryIndex;
        return;
    }
    EntryIndex = *reinterpret_cast<const int32*>(Entry + 0x10);
}
OutIndex = -1;
// CONFIDENCE: MEDIUM
