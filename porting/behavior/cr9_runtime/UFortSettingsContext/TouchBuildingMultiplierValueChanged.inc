// Original body 0x144d20e80 (thunk 0x1452d27d0). Offset evidence: settings record from NativeGetSettingsRecord144d03d50 (0x144d03d50) at +0x32c = TouchBuildingMultiplier (reflected Float, exact lookup); record +0x87e = bDirty (reflected bool). UCOMISS sets ZF=0 only for ordered unequal operands, and only that case sets bDirty before the store.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record != nullptr)
{
    if (NewValue < Record->TouchBuildingMultiplier || NewValue > Record->TouchBuildingMultiplier)
    {
        Record->bDirty = true;
    }
    Record->TouchBuildingMultiplier = NewValue;
}
