// Behaviour snippet: AFortPlayerPawn::GetSlopeSlidingNormal (body only).
// Original body address: 0x1423247c0 (thunk 0x142a7c9d0). Writes the FVector return through the hidden return pointer.
// Offset evidence: [this+0x10e8] bit 0x4 = bIsSlopeSliding (reflected bitfield, AFortPlayerPawn). [this+0x280] = CharacterMovement (ACharacter, private: GetCharacterMovement()).
// The FVector at CharacterMovement+0x1e20 has no reflected pointee; read raw (X,Y as qword, Z as dword). Fallback (0,0,1) from writable snapshot globals 0x147c31cb0/0x147c31cb8.
// CONFIDENCE: MEDIUM
if (!bIsSlopeSliding)
{
	return FVector(0.0f, 0.0f, 1.0f);
}
const uint8* MovementRaw = reinterpret_cast<const uint8*>(GetCharacterMovement());
return *reinterpret_cast<const FVector*>(MovementRaw + 0x1e20);
