// Helper body for UFortStreamMediaSource static helper at 0x141e2b6c0 (asm: MOVZX EAX,byte ptr [RCX + 0x57e]; RET). No branches, no calls.
// Offset evidence: lookup_offset UFortStreamMediaSource 0x57e reports no reflected member.
// Caller: UFortStreamMediaSource::ShouldProtectPlayerFromContent 0x141f6fcf0 passes the non-null result of CALL 0x14237e580 as RCX, then tests AL.
// The raw byte is returned; the bool conversion keeps the same TEST AL,AL outcome for callers.
// CONFIDENCE: HIGH
return *reinterpret_cast<const uint8*>(reinterpret_cast<const uint8*>(MediaSrc) + 0x57e) != 0;
