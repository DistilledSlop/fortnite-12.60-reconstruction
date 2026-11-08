// Original body 0x144d20ec0 (thunk 0x1452d2850). Offset evidence: settings record from NativeGetSettingsRecord144d03d50 (0x144d03d50) at +0x328 = TouchScopedSlow (reflected Float, exact lookup); record +0x87e = bDirty (reflected bool). UCOMISS sets ZF=0 only for ordered unequal operands, and only that case sets bDirty before the store.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record != nullptr)
{
    if (NewValue < Record->TouchScopedSlow || NewValue > Record->TouchScopedSlow)
    {
        Record->bDirty = true;
    }
    Record->TouchScopedSlow = NewValue;
}
