// Original body 0x144d21000 (thunk 0x1452d2ad0). Offset evidence: settings record from NativeGetSettingsRecord144d03d50 (0x144d03d50) at +0x334 = TouchVerticalMultiplier (reflected Float, exact lookup); record +0x87e = bDirty (reflected bool). UCOMISS sets ZF=0 only for ordered unequal operands, and only that case sets bDirty before the store.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record != nullptr)
{
    if (NewValue < Record->TouchVerticalMultiplier || NewValue > Record->TouchVerticalMultiplier)
    {
        Record->bDirty = true;
    }
    Record->TouchVerticalMultiplier = NewValue;
}
