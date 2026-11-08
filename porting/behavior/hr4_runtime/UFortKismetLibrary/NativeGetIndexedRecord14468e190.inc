// Helper 0x14468e190 (small pool, host UFortKismetLibrary). Callers: 137.
// Asm evidence: MOVSXD RAX,[RCX+0x168] is a signed int32 index; CMP EAX,[RCX+0x160] is a signed int32 count (JGE);
//   IMUL RAX,RAX,0x78 is the record stride; ADD RAX,[RCX+0x158] adds the record data pointer.
// Otherwise (index < 0 or index >= count) the helper returns NULL (XOR EAX,EAX).
// Sampled callers (native-cap-tailjmp0/0x142478380, native-cap-direct/0x141acd4b0) pass RCX=[this+0xb8],
//   and compare the returned pointer against NULL and against a second pointer loaded from [this+0x248].
// The owner's concrete class is not justified by the callers, so raw offsets are used through UObject*.
// CONFIDENCE: MEDIUM
const int32 RecordIndex = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(Owner) + 0x168);
const int32 RecordCount = *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(Owner) + 0x160);
if (RecordIndex < 0 || RecordIndex >= RecordCount)
{
	return nullptr;
}
uint8* const RecordData = *reinterpret_cast<uint8* const*>(reinterpret_cast<const uint8*>(Owner) + 0x158);
return RecordData + static_cast<int64>(RecordIndex) * 0x78;
