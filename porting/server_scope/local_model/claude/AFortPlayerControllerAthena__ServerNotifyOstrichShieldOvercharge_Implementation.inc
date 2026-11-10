// CONFIDENCE: MEDIUM
// Original target 0x1419f2080.
auto* PawnRef = Pawn;	// this+0x248
if (PawnRef != nullptr && PawnRef->IsA(AFortPlayerPawnAthena::StaticClass()))
{
	auto* Ostrich = Helper_142325510(PawnRef);	// pawn+0x1578
	if (Ostrich != nullptr && Ostrich->IsA(AFortCharacterVehicle_Ostrich::StaticClass()))
	{
		// FUN_141f08e90 checks the Ostrich's +0x3510 value against this controller.
		if (Helper_141f08e90(Ostrich->Value_0x3510, this))
		{
			Helper_141952090(Ostrich);	// applies the shield overcharge (sets a 1.0 value through FUN_1424b3430)
			Helper_142860150(Ostrich);	// calls the Ostrich's BP event (FindFunctionChecked + ProcessEvent)
		}
	}
}
