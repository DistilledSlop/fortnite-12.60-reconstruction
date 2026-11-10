// CONFIDENCE: MEDIUM - control flow fully read; the flag bytes at +0xB20, +0x1062 and weapon +0xBD4 are unreflected and carry the names below by position only.
// Original address: 0x142a48490 (exec thunk, bool return stored as one byte). Helper: 0x1423908a0. Weapon class: 0x142b4dac0 (FortWeaponRanged). Bit read: 0x1426e7770.
// Base value is the flag byte at +0xB20 (bit 0 is the reported flag). It gains bit 0 when the pawn's current ranged weapon has its
// flag bit 0x20 (weapon +0xBD4) set and the pawn has its flag bit 0x10 (+0x1062) set.
uint8 Flags = UnknownFlags_B20;
if (MyFortPawn != nullptr)
{
	AFortWeapon* Weapon = MyFortPawn->CurrentWeapon;
	if (Weapon != nullptr && Weapon->IsA(AFortWeaponRanged::StaticClass()))
	{
		const bool bWeaponFlag = (Weapon->UnknownFlags_BD4 & 0x20) != 0;
		const bool bPawnFlag = (MyFortPawn->UnknownFlags_1062 & 0x10) != 0;
		if (bWeaponFlag && bPawnFlag)
		{
			Flags |= 1;
		}
	}
}
return Flags;
