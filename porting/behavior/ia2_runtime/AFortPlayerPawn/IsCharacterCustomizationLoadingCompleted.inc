// Behaviour snippet: AFortPlayerPawn::IsCharacterCustomizationLoadingCompleted (inline thunk body; spliced into the generated member function).
// Original body address: 0x142a7d0b0 (thunk is body, group A, this_delta_total 0)
// Offset evidence: MOV EAX,dword [object+0x2004]; AND AL,0x18; CMP AL,0x10; SETZ = (low byte of dword at 0x2004 & 0x18) == 0x10.
// lookup_offset.py: 0x2004 is not covered by any single reflected member (0x1ff8 SpawnSound ends at 0x2000). Declared bitfield bools bDamagedEnemy (0x40) and bIsLocalPlayer (0x2) share the dword but not bits 0x18, so raw access.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
const int32 Word = *reinterpret_cast<const int32*>(Raw + 0x2004);
return (static_cast<uint8>(Word) & 0x18) == 0x10;
