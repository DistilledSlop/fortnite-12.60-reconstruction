// Original body address: 0x141aaed40
// Evidence: MOV RAX,[RCX+0x260] is CachedGameState (reflected Object, inherited from AFortAthenaMutator). Pointee int32 at +0x11e4 has no reflected member in AFortGameStateAthena; raw read. Null CachedGameState returns 0.0f.
// Verbosity-gated diagnostic log at call 0x142c54b70 elided (no state change).
// CONFIDENCE: MEDIUM
if (CachedGameState == nullptr)
{
    return 0.0f;
}
const uint8* GameStateRaw = reinterpret_cast<const uint8*>(CachedGameState);
return static_cast<float>(*reinterpret_cast<const int32*>(GameStateRaw + 0x11e4));
