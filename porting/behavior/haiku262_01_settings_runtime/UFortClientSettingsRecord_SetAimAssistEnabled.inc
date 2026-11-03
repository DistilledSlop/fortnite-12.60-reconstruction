// Native0x141e39a60 CONFIDENCE: high
// vfn +0x150 is UObject::GetWorld() (decompilation_notes slot 42, HIGH); result discarded. Stub_ReturnFalse() (folded constant false) selects the non-mobile field.
GetWorld();
if (bAimAssistEnabled != bNewValue)
{
    bAimAssistEnabled = bNewValue;
    bDirty = true;
}
