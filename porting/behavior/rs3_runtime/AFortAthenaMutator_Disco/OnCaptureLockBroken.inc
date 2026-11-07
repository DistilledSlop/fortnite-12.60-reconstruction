// Original body address: 0x141adc6d0 (thunk 0x1427fe7d0)
// Offset evidence: CapturePoint (RDX) test bit 0 of byte +0x9f8 = AAthenaCapturePoint::bActivated. LockTeam = R8B, BreakTeam = R9B (ABI order).
// this+0x260 = CachedGameState; GameState+0x1f00..0x1f0c = MutatorEventData {EventId, EventParam1, EventParam2, EventParam3} (offsets 0,4,8,12).
// No null check on CachedGameState (asm writes through it directly).
// CONFIDENCE: HIGH
if (CapturePoint == nullptr || !CapturePoint->bActivated)
{
    return;
}
CachedGameState->MutatorEventData.EventId = 4;
CachedGameState->MutatorEventData.EventParam1 = LockTeam;
CachedGameState->MutatorEventData.EventParam2 = BreakTeam;
++CachedGameState->MutatorEventData.EventParam3;
