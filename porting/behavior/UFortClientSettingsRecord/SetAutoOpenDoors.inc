// Native0x141e3a6e0 CONFIDENCE: high
// helper returns -Stub (always 0); reflected function is void so the constant return is not part of the contract
// vfn +0x150 is UObject::GetWorld() (decompilation_notes slot 42, HIGH); result discarded. Stub_ReturnFalse() (folded constant false) selects the non-mobile field.
GetWorld();
if (bAutoOpenDoorsNonMobile != bNewValue)
{
    bAutoOpenDoorsNonMobile = bNewValue;
    bDirty = true;
}
