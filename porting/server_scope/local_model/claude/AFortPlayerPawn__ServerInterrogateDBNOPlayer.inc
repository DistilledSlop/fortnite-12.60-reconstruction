// CONFIDENCE: MEDIUM; thunk 0x142a7f380 has no validate and calls impl 0x14234a380 directly. Impl: if InDBNOHoistee is set, the Controller-IsA-AFortPlayerControllerAthena branch calls Stub_Empty() (decompiler-named empty stub, address not resolved in capture, no effect assumed), then starts interrogation of the target with bIsInterrogating=true via FUN_14232d220 (its body is the same record-hoist logic as Hoist with byte +0x3ea set to the flag; 0x142336990 not read to depth two).
if (InDBNOHoistee)
{
	AFortPlayerControllerAthena* AthenaController = Cast<AFortPlayerControllerAthena>(Controller);
	if (AthenaController)
	{
		// Stub_Empty(): no effect.
	}
	BeginDBNOHoistOrInterrogate(InDBNOHoistee, this, /*bIsInterrogating=*/true);   // FUN_14232d220(hoistee, this, 1)
}
