// Original body address: 0x141b2ad70 (thunk 0x1428200a0)
// Offset evidence: this+0xf0 = AActor::Role (Byte) compared with 3 (ROLE_Authority). CALL 0x1420e5530 = AFortGameplayMutator::IsMutatorActive() const
// on this. NumPlayersRequested (EDX) is added to dword this+0x338, which no reflected member covers (member_field NativeRecoveredInt32_0x338).
// CONFIDENCE: HIGH
if (static_cast<uint8>(GetLocalRole()) != 3)
{
    return;
}
if (!IsMutatorActive())
{
    return;
}
NativeRecoveredInt32_0x338 += NumPlayersRequested;
