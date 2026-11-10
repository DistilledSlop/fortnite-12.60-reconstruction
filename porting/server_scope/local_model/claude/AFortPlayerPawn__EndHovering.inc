// CONFIDENCE: MEDIUM
// Original address: 0x142a7b5d0 (exec thunk, no parameters). Capture: captures/13799d83931e48b19e2b5ccf2a1528b5/0x142a7b5d0.c
if (Role != ROLE_SimulatedProxy)
{
	bIsUsingJetpack = false;   // BalloonActiveCount byte at 4329 &= ~0x10
	if (VehicleStateLocal.Vehicle == nullptr)   // first qword of VehicleStateLocal (offset 5496), name inferred
	{
		UFortMovementComp_Character* Movement = Cast<UFortMovementComp_Character>(CharacterMovement);
		if (Movement != nullptr && Movement->MovementMode == MOVE_Custom && Movement->CustomMovementMode == 6)   // byte +0x168 and +0x169
		{
			// Slot 0x590 with (MOVE_Falling = 3). The custom-mode argument is not passed.
			Movement->SetMovementMode(MOVE_Falling);
		}
	}
}
