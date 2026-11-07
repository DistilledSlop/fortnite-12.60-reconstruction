// Original address: 0x141add140 (AFortAthenaMutator_GameModeBase::OnGamePhaseChanged)
// Offset evidence: [RCX+0xf0] is Role (reflected); [RCX+0x260] is CachedGameState (reflected Object); int32 1000 written raw at +0x1f00.
// The asm has no null check on CachedGameState and none is added here.
// CONFIDENCE: HIGH
if (static_cast<uint8>(Role) != 3 || static_cast<uint8>(NewPhase) != 3)
{
    return;
}
*reinterpret_cast<int32*>(reinterpret_cast<uint8*>(CachedGameState) + 0x1f00) = 1000;
