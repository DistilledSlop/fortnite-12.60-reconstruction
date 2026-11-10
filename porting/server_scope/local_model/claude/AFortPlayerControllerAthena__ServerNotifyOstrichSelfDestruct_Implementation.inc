// CONFIDENCE: MEDIUM
// Original target 0x1419f1fc0.
auto* PawnRef = Pawn;	// this+0x248
if (PawnRef == nullptr || !PawnRef->IsA(AFortPlayerPawnAthena::StaticClass()))
{
	return;
}
// The pawn itself is an Ostrich: self-destruct on the pawn (mode 0).
if (PawnRef->IsA(AFortCharacterVehicle_Ostrich::StaticClass()))
{
	Helper_1419729a0(PawnRef, 0);
	return;
}
// Otherwise the pawn's Ostrich (pawn+0x1578, FUN_142325510) must be an Ostrich: self-destruct on it (mode 1).
auto* Ostrich = Helper_142325510(PawnRef);
if (Ostrich != nullptr && Ostrich->IsA(AFortCharacterVehicle_Ostrich::StaticClass()))
{
	Helper_1419729a0(Ostrich, 1);
}
