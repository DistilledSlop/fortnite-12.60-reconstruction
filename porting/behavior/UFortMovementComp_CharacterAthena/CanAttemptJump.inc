// Original address: 0x1419c2ad0
// CONFIDENCE: high
return IsJumpAllowed()
	&& !bWantsToCrouch
	&& (IsMovingOnGround() || IsFalling() || (MovementMode == MOVE_Custom && CustomMovementMode == 10));
