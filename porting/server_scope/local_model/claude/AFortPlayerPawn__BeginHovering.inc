// CONFIDENCE: HIGH
// Original address: 0x142a7aec0 (exec thunk, no parameters). Capture: captures/13799d83931e48b19e2b5ccf2a1528b5/0x142a7aec0.c
// Role byte is at offset 0xf0 (AActor::Role). Role == 1 is ROLE_SimulatedProxy.
// Movement class check uses FUN_1429f5d30 = UFortMovementComp_Character::StaticClass (named from its GetPrivateStaticClassBody string).
if (Role != ROLE_SimulatedProxy)
{
	bIsUsingJetpack = true;   // BalloonActiveCount byte at 4329 |= 0x10 (bIsUsingJetpack)
	UFortMovementComp_Character* Movement = Cast<UFortMovementComp_Character>(CharacterMovement);   // ACharacter::CharacterMovement, offset 0x280
	if (Movement != nullptr)
	{
		// SetMovementMode(MOVE_Custom, 6): custom mode 6 is hover. Slot 0x590 is named SetMovementMode here, unverified (header model names it IsMoveInputIgnored).
		Movement->SetMovementMode(MOVE_Custom, 6);
	}
}
