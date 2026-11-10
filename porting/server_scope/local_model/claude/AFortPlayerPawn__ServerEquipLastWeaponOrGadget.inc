// CONFIDENCE: HIGH for control flow, MEDIUM for helper semantics. Validate (vcall 0xf78) is the shared return-true stub 0x1406801d0, so it always passes. Impl 0x14234a320 calls FUN_142314d70(this, bForce) (rcx=this, dl=bForce per disassembly) and, if that returned false and bForce is set, calls EquipBestWeapon (vcall 0xbc0 = AFortPawn::EquipBestWeapon).
const bool bEquippedLast = EquipLastWeaponOrGadgetInternal(bForce);
if (!bEquippedLast && bForce)
{
	EquipBestWeapon();
}
