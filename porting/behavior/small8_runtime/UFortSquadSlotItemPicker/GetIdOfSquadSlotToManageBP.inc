// Original body address: 0x145119db0
// Offset evidence: FName at object offset 0x330 and int32 at 0x338. Both lie below this class lowest own reflected offset 0x368 and no reflected member covers them, so raw access is used.
// CONFIDENCE: MEDIUM
const uint8* Raw = reinterpret_cast<const uint8*>(this);
OutSquadId = *reinterpret_cast<const FName*>(Raw + 0x330);
OutSquadSlotIndex = *reinterpret_cast<const int32*>(Raw + 0x338);
