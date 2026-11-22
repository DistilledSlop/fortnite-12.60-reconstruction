// Original address: 0x1419c2ad0
// CONFIDENCE: high
const AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner);
return (!Player || !Player->bIsDBNO)
	&& IsJumpAllowed()
	&& !bWantsToCrouch
	&& (IsMovingOnGround() || IsFalling() || (MovementMode == MOVE_Custom && CustomMovementMode == 10));
