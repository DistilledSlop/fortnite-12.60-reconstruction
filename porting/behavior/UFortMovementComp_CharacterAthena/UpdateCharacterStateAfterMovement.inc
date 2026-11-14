// Original address: 0x1419fff00
// CONFIDENCE: high
Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
NativeMeleeSlowRemaining1e5c = FMath::Max(NativeMeleeSlowRemaining1e5c - DeltaSeconds, 0.0f);
