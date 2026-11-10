// CONFIDENCE: MEDIUM
// Original addresses: exec thunk 0x1428f0350 -> FUN_141949f60(Context, &Result, DefaultVehicleClass); result = *Result.
// Override table: AthenaVehicleOverrides (this+0x1330, element stride 0x60, count at this+0x1338).
UClass* ResultClass = DefaultVehicleClass;
for (const FAthenaVehicleOverride& Override : AthenaVehicleOverrides)
{
	// Entries that are active for this world are skipped (state >= 2, world object valid, and the entry is not overridden).
	// Everything else falls through to the match test (0x143f76130 / 0x1428ea7f0 / 0x141bd9690 / 0x142261550 chain).
	if (Override.State >= 2 && GetOverrideOwner(this) != nullptr && IsEntryActiveForWorld(Override, this))
	{
		continue;
	}
	// Resolve the weak key object (FWeakObjectPtr_Get); on a stale key the port treats it as null, matching the decompile.
	UObject* KeyObject = Override.Key.Get();
	if (KeyObject == DefaultVehicleClass)
	{
		// Matched: return the override value when it is a vehicle class, otherwise null (decompile sets the result to 0).
		ResultClass = IsValidVehicleOverrideValue(Override.Value) ? Override.Value : nullptr;
		break;
	}
}
return ResultClass;
