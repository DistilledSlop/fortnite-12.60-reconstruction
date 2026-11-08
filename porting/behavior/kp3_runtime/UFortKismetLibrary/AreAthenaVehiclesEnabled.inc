// Behaviour snippet: UFortKismetLibrary::AreAthenaVehiclesEnabled (body only).
// Original address: 0x141ee5f10 (thunk 0x142966db0)
// Offset evidence: UFortGlobals bool members at object +0x213 (bAthenaSimVehicles, reflected bool_bits mask 1) and +0x212 (bAthenaUsesSimCollision, reflected). Helper 0x141ed2350 = UFortGlobals::NativeGetGlobals(), called once per read exactly as the asm does.
// CONFIDENCE: HIGH
if (!UFortGlobals::NativeGetGlobals()->bAthenaSimVehicles)
{
	return false;
}
return UFortGlobals::NativeGetGlobals()->bAthenaUsesSimCollision;
