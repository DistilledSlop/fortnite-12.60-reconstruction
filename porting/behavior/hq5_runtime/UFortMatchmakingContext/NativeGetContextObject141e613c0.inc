// Helper 0x141e613c0: plain pointer load at +0x438.
// lookup_offset.py UFortMatchmakingContext 0x438 returns no reflected member, so raw access is used.
// Confidence MEDIUM: the stored pointee type is not established; callers read byte +0x118 of the result.
// CONFIDENCE: MEDIUM
return *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(Context) + 0x438);
