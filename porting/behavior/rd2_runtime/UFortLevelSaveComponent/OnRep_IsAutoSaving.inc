// Behaviour snippet: UFortLevelSaveComponent::OnRep_IsAutoSaving (body only; spliced into the generated member function).
// Original address: 0x141efd950 (thunk 0x14297f1e0). Asm: porting/generated/native-cap-direct/0x141efd950.asm
// Offset evidence: bIsAutoSaving = bitfield at +0x3b1 (mask 0x1, reflected); the byte test (CMP byte,0) is taken as bIsAutoSaving.
// Offset evidence: OnClientSaveStateChanged = FOnSavingStateChanged (one bool param) at +0x190; its invocation-list Num (int32) at +0x198 gates the broadcast (JLE skip).
// Verbosity-gated diagnostic log at 0x142c54b70 (guarded by byte [0x148133ad0] >= 5, both category branches) elided (no state change)
// CONFIDENCE: MEDIUM
if (*reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(this) + 0x198) > 0)
{
    OnClientSaveStateChanged.Broadcast(bIsAutoSaving);
}
