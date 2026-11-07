// Original body address: 0x144de1670 (thunk 0x1453005a0). Offset evidence: PreregisteredTabInfoArray is the reflected TArray<FFortTabListRegistrationInfo> at object +0x350 (Num at +0x358); element stride 0xc8 equals sizeof(FFortTabListRegistrationInfo). Match is TabNameId (FName at element +0); the store is bHidden (byte at element +0x8).
// CONFIDENCE: HIGH
for (FFortTabListRegistrationInfo& Info : PreregisteredTabInfoArray)
{
	if (Info.TabNameId == TabNameId)
	{
		Info.bHidden = bHidden;
		return;
	}
}
