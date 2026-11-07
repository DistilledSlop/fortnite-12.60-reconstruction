// Original body address: 0x1422ec070 (thunk 0x1427a5930)
// Offset evidence: +0xf0 is AActor::Role (engine storage, read raw; 3 is ROLE_Authority). CurrentAIRotationRate (FRotator) at +0x1a38.
// The asm writes pitch/yaw (8 bytes) and roll (4 bytes at +0x1a40), a full 12-byte FRotator copy.
// ACharacter::CharacterMovement at +0x280 (engine, raw); its FRotator RotationRate at +0x1e4 gets the same 12-byte copy.
// MEDIUM: UCharacterMovementComponent is not generated here, so RotationRate is taken as the engine FRotator from the 12-byte width.
// CONFIDENCE: MEDIUM
uint8* Raw = reinterpret_cast<uint8*>(this);
if (Raw[0xf0] != 3)
{
	return;
}
CurrentAIRotationRate = AIRotationRate;
uint8* MovementRaw = *reinterpret_cast<uint8**>(Raw + 0x280);
if (MovementRaw != nullptr)
{
	*reinterpret_cast<FRotator*>(MovementRaw + 0x1e4) = AIRotationRate;
}
