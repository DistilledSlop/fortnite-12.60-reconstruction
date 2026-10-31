// Behaviour snippet: FSharedRepMovement::NetSerialize (body only; spliced inline into the struct header, kind struct_net).
// Original address: 0x141a1b9d0
// Decompile source: porting/decompilation_notes.md, Eleventh pass (slot-14 thunk and body, Ghidra probe and asm).
// Thunk note: reached from the ops thunk 0x142a82170 (MOV RCX,[RSP+0x28]; JMP). Source: Ghidra slot-14 decompile and objdump of the dump.
// Verified from the asm: the archive call sequence and each destination, the quantizer arguments, and the rotator choice.
// CONFIDENCE: HIGH
bOutSuccess = true;

// FRepMovement block: engine FRepMovement::NetSerialize (2 flag bits, Location +0x18 by +0x31, rotator +0x24 by +0x33,
// LinearVelocity +0x00 by +0x32, AngularVelocity +0x0c by +0x32 only when bRepPhysics).
RepMovement.NetSerialize(Ar, Map, bOutSuccess);

Ar.Serialize(&RepMovementMode, 1);            // +0x47 uint8, 8 bits
Ar.Serialize(&JumpFlashCountPacked, 1);       // +0x48 uint8, 8 bits
Ar.Serialize(&LandingFlashCountPacked, 1);    // +0x49 uint8, 8 bits
Ar.Serialize(&CurrentMovementStyle, 1);       // +0x4a enum byte, 8 bits

// Ten bools in ascending offset order, each as a 32-bit UBOOL (FArchive::SerializeBool).
Ar.SerializeBool(bProxyIsJumpForceApplied);   // +0x4b
Ar.SerializeBool(bIsCrouched);                // +0x4c
Ar.SerializeBool(bIsSkydiving);               // +0x4d
Ar.SerializeBool(bIsParachuteOpen);           // +0x4e
Ar.SerializeBool(bIsSlopeSliding);            // +0x4f
Ar.SerializeBool(bIsProxySimulationTimedOut); // +0x50
Ar.SerializeBool(bIsTargeting);               // +0x51
Ar.SerializeBool(bIsWaterJump);               // +0x52
Ar.SerializeBool(bIsWaterSprintBoost);        // +0x53
Ar.SerializeBool(bIsWaterSprintBoostPending); // +0x54

{
	// RepTimeStamp (+0x34): 1 bit (nonzero), then the float when it is set; otherwise it becomes +0.0.
	uint8 bHas = RepTimeStamp != 0.0f ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&RepTimeStamp, 4);
	}
	else
	{
		RepTimeStamp = 0.0f;
	}
}

Ar.Serialize(&RemoteViewData32, 4);          // +0x40 uint32, always present, 32 bits

{
	// AccelerationPack (+0x44): 1 bit, then uint16 when set.
	uint8 bHas = AccelerationPack != 0 ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&AccelerationPack, 2);
	}
	else
	{
		AccelerationPack = 0;
	}
}

{
	// AccelerationZPack (+0x46, int8): 1 bit, then 8 bits when set.
	uint8 bHas = AccelerationZPack != 0 ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&AccelerationZPack, 1);
	}
	else
	{
		AccelerationZPack = 0;
	}
}

{
	// TurretYaw (+0x38) and TurretPitch (+0x3c): 1 bit (either nonzero), then both floats when set.
	uint8 bHas = (TurretYaw == 0.0f && TurretPitch == 0.0f) ? 0 : 1;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&TurretYaw, 4);
		Ar.Serialize(&TurretPitch, 4);
	}
	else
	{
		TurretYaw = 0.0f;
		TurretPitch = 0.0f;
	}
}

return true;
