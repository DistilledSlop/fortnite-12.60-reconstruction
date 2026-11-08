// Original body 0x144e485f0 (thunk 0x145300040). Evidence: reflected member OwningPedestalGroup at +0x380 (Object); null returns true; otherwise NativeHelper144e0d6f0 result (AL) is returned.
// CONFIDENCE: HIGH
static_assert(sizeof(AAthenaSquadPedestalGroup) > 0, "AAthenaSquadPedestalGroup must be complete for the group pointer conversion.");
if (!OwningPedestalGroup)
{
	return true;
}
return AFortTeamMemberPedestal::NativeHelper144e0d6f0(OwningPedestalGroup);
