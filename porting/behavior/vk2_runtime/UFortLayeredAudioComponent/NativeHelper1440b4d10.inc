// Native helper 0x1440b4d10: virtual slot 0x540 of UFortLayeredAudioComponent (member, const).
// Asm: MOVZX EAX,byte ptr [RCX + 0x86]; AND AL,0x1; RET.
// Offset 0x86 is not covered by any reflected member (lookup_offset.py UFortLayeredAudioComponent 0x86), so raw read.
// Returns bit 0 of the byte at +0x86. Semantic name unknown; caller evidence not located.
// CONFIDENCE: MEDIUM
return (*reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(this) + 0x86) & 0x1) != 0;
