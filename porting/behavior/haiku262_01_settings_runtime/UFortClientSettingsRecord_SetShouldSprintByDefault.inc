// Native0x141e3c820 CONFIDENCE: high
// vfn +0x150 is UObject::GetWorld() (decompilation_notes slot 42, HIGH); result discarded. Stub_ReturnFalse() (folded constant false) selects the non-mobile field.
GetWorld();
if (bSprintByDefault != bShouldSprintByDefault)
{
    bSprintByDefault = bShouldSprintByDefault;
    bDirty = true;
}
