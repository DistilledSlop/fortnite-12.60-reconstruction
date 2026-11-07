// Original body address: 0x145119d90
// Offset evidence: FName at object offset 0x328 and int32 at 0x330. The class has no reflected members of its own and no ancestor member covers these offsets, so raw access is used.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
OutSquadId = *reinterpret_cast<const FName*>(Raw + 0x328);
OutSquadSlotIndex = *reinterpret_cast<const int32*>(Raw + 0x330);
