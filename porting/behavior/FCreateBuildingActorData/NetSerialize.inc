// Behaviour snippet: FCreateBuildingActorData::NetSerialize (body only; spliced inline into the struct header, kind struct_net).
// Original address: 0x1423953d0
// Decompile source: porting/decompilation_notes.md, Eleventh pass (slot-14 thunk and body, Ghidra probe and asm).
// Thunk note: reached from the ops thunk 0x142a3dc40. Layout from the asm: SerializeInt(handle, 511),
// bool as UBOOL, BuildLoc as raw floats, BuildingClassData.UpgradeLevel (+0x34), then saving or loading SyncKey
// (SerializeInt over 2^24 with the 13x quantizer) and a 2-bit yaw code in one byte. BuildRot Pitch and Roll are never serialized.
// The wrap helper 0x142bc0d80 is re-expressed as clamp(Yaw - trunc(Yaw/360)*360, -360, 360) (MEDIUM).
// CONFIDENCE: MEDIUM (layout and constants HIGH; the wrap of the saving yaw is MEDIUM)
Ar.SerializeInt(BuildingClassHandle, 0x1ff);
Ar.SerializeBool(bMirrored);
Ar.Serialize(&BuildLoc.X, 4);
Ar.Serialize(&BuildLoc.Y, 4);
Ar.Serialize(&BuildLoc.Z, 4);
Ar.Serialize(&BuildingClassData.UpgradeLevel, 1);

if (Ar.IsSaving())
{
	// SyncKey (0x1423642d0): ((round_half_even(2 * (13 * x) + 0.5)) >> 1) + 0x800000, clamped to [0, 0xffffff].
	const float Scaled = SyncKey * 13.0f;
	const float Doubled = Scaled + Scaled + 0.5f;
	int32 Biased = (static_cast<int32>(::nearbyintf(Doubled)) >> 1) + 0x800000;
	if (static_cast<uint32>(Biased) > 0xffffff)
	{
		Biased = (Biased > 0) ? 0xffffff : 0;
	}
	uint32 SyncCode = static_cast<uint32>(Biased);
	Ar.SerializeInt(SyncCode, 0x1000000);

	// Yaw code (0x142bc0d80 with 360.0f, then the bins of 0x1423953d0): 0 = (90, 180], 1 = (0, 90], 2 = (-90, 0], 3 = [-180, -90].
	float Quotient = BuildRot.Yaw / 360.0f;
	if (FMath::Abs(Quotient) < 8388608.0f)
	{
		Quotient = static_cast<float>(static_cast<int32>(Quotient));
	}
	float Mid = Quotient * 360.0f;
	if (FMath::Abs(BuildRot.Yaw) < FMath::Abs(Mid))
	{
		Mid = BuildRot.Yaw;
	}
	float Wrapped = BuildRot.Yaw - Mid;
	if (Wrapped < -360.0f)
	{
		Wrapped = -360.0f;
	}
	else if (Wrapped > 360.0f)
	{
		Wrapped = 360.0f;
	}
	if (Wrapped < 0.0f)
	{
		Wrapped = Wrapped + 360.0f;
	}
	if (Wrapped > 180.0f)
	{
		Wrapped -= 360.0f;
	}
	uint8 YawCode = 0;
	if (Wrapped <= 90.0f)
	{
		if (Wrapped <= 0.0f)
		{
			YawCode = (Wrapped <= -90.0f) ? 3 : 2;
		}
		else
		{
			YawCode = 1;
		}
	}
	Ar.Serialize(&YawCode, 1);
}
else
{
	uint32 SyncCode = 0;
	Ar.SerializeInt(SyncCode, 0x1000000);
	SyncKey = static_cast<float>(static_cast<int32>(SyncCode) - 0x800000) * 0.07692308f;

	uint8 YawCode = 0;
	Ar.Serialize(&YawCode, 1);
	BuildRot.Yaw = (YawCode == 0) ? 180.0f : ((YawCode == 1) ? 90.0f : ((YawCode == 2) ? 0.0f : -90.0f));
}

bOutSuccess = true;
return true;
