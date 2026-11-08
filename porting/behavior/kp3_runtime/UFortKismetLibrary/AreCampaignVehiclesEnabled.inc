// Behaviour snippet: UFortKismetLibrary::AreCampaignVehiclesEnabled (body only).
// Original address: 0x141ee5f40 (thunk 0x142966de0)
// Offset evidence: UFortGlobals bool members at object +0x217 (bCampaignSimVehicles, reflected bool_bits mask 1) and +0x216 (bCampaignUsesSimCollision, reflected). Helper 0x141ed2350 = UFortGlobals::NativeGetGlobals(), called once per read exactly as the asm does.
// CONFIDENCE: HIGH
if (!UFortGlobals::NativeGetGlobals()->bCampaignSimVehicles)
{
	return false;
}
return UFortGlobals::NativeGetGlobals()->bCampaignUsesSimCollision;
