// Behaviour snippet: FFortAISharedRepMovement::NetSerialize (body only; spliced inline into the struct header, kind struct_net).
// Original address: 0x1422df100
// Decompile source: porting/decompilation_notes.md, Eleventh pass (slot-14 thunk and body, Ghidra probe and asm).
// Thunk note: reached from the ops thunk 0x1427a19f0 (MOV RCX,[RSP+0x28]; JMP).
// Verified from the asm: archive call sequence and destinations (+0x3c, +0x40, +0x3d order), default store on load.
// CONFIDENCE: HIGH
bOutSuccess = true;

// FRepMovement block: same pattern as FSharedRepMovement (2 flag bits, Location, rotator, linear and angular velocity).
RepMovement.NetSerialize(Ar, Map, bOutSuccess);

Ar.Serialize(&RepMovementMode, 1);            // +0x38 uint8
Ar.Serialize(&RepCurrentFortAILODLevel, 1);   // +0x39 EFortAILODLevel (uint8)

{
	// RepTimeStamp (+0x34): 1 bit (nonzero), then the float when set; otherwise +0.0.
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

{
	// RepSharedAnimInfo (8 bytes at +0x3c): 1 bit (MontageIndex != -1). When set the wire order is
	// AnimState (+0x3c, 8 bits), MontageIndex (+0x40, 32 bits), MontageSectionToPlay (+0x3d, 8 bits).
	// When the bit is clear and the archive is loading, the body stores AnimState = Anim_MAX (0x0b),
	// MontageSectionToPlay = 0 and MontageIndex = -1 (the asm stores 0xffffffff0000000b over +0x3c..+0x43).
	uint8 bHas = RepSharedAnimInfo.MontageIndex != -1 ? 1 : 0;
	Ar.SerializeBits(&bHas, 1);
	if (bHas)
	{
		Ar.Serialize(&RepSharedAnimInfo.AnimState, 1);
		Ar.Serialize(&RepSharedAnimInfo.MontageIndex, 4);
		Ar.Serialize(&RepSharedAnimInfo.MontageSectionToPlay, 1);
	}
	else if (Ar.IsLoading())
	{
		RepSharedAnimInfo.AnimState = EFortSharedAnimationState::Anim_MAX;
		RepSharedAnimInfo.MontageSectionToPlay = 0;
		RepSharedAnimInfo.MontageIndex = -1;
	}
}

return true;
