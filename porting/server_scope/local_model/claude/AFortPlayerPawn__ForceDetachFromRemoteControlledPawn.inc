// CONFIDENCE: MEDIUM
// Original address: 0x142a7b880 (exec thunk, no parameters). Captures: 0x142a7b880 (this function) and 0x14243a470 (FUN_14243a470, read in full, inlined below).
if (Role == ROLE_Authority && FWeakObjectPtr_IsValid(ControlledRCPawn))
{
	AFortPlayerPawn* RCPawn = Cast<AFortPlayerPawn>(FWeakObjectPtr_Get(ControlledRCPawn));
	if (RCPawn != nullptr && RCPawn->Controller != nullptr && IsA<AFortPlayerControllerAthena>(RCPawn->Controller))
	{
		// The inlined body of FUN_14243a470 (the detach helper). Its weak "remote controlled pawn" is at +0x30a4 (offset 12452).
		if (FWeakObjectPtr_IsValid(RCPawn->RemoteControlledPawnWeak))
		{
			RCPawn->OnDetachBegin();                                   // vslot 0x5e8, unnamed
			AFortPawn* Controlled = FWeakObjectPtr_Get(RCPawn->RemoteControlledPawnWeak);
			Controlled->OnDetachBegin();                               // vslot 0x5e8, unnamed
			Controlled = FWeakObjectPtr_Get(RCPawn->RemoteControlledPawnWeak);
			UObject* ControlledComponent = Controlled->GetOwnedComponent();   // vtable 0x10 of Controlled->[0x1e], unnamed
			if (ControlledComponent != nullptr && RCPawn->SeatIndex != -1)   // RCPawn +0xd10 (offset 0x1110 / 0x1111) seat index, unnamed
			{
				ControlledComponent->SetSeatState(RCPawn->SeatIndex, -1);   // vslot 0x5c0, unnamed
			}
			if (Controlled->SomeFlags != 0)
			{
				Controlled->SomeFlags &= ~0x04;   // byte at +0x1f1 of the controlled object, unnamed
			}
			SetControlledInputBlocked(Controlled, false);                   // FUN_14234bec0
			SetPossessionTransition(Controlled, true);                     // FUN_14234b560 (1)
			ReassignController(RCPawn->Controller, Controlled);             // FUN_14416eac0
			SetPossessionTransition(Controlled, false);                    // FUN_14234b560 (0)
			bool bIsInVehicle = IsInVehicleQuery(Controlled);              // FUN_1423318f0
			if (bIsInVehicle && Controlled->MovementComponentOrNull() != nullptr)   // *(+0x80 of the controlled object)
			{
				UFortMovementComp_Character* Movement = Cast<UFortMovementComp_Character>(Controlled->MovementComponentOrNull());
				if (Movement != nullptr)
				{
					bool bVehicleSeatFlag = IsVehicleSeatFlagged(Controlled);   // FUN_142331180
					Movement->SetMovementMode(MOVE_Custom, bVehicleSeatFlag ? 1 : 2);   // slot 0x590 with (6, (cVar3 == 0) + 1)
				}
			}
			RCPawn->PreviousRemoteControlledPawnWeak = RCPawn->RemoteControlledPawnWeak;   // +0x30ac = +0x30a4
			RemoteControlledPawnWeak_Reset(RCPawn->RemoteControlledPawnWeak);   // FUN_142ee3b30(&tmp, 0)
		}
	}
}
