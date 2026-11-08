// Original body 0x144d20f40 (thunk 0x1452d2950). Offset evidence: settings record from NativeGetSettingsRecord144d03d50 (0x144d03d50) at +0x31c = TouchLookAccelerationMultiplier (reflected Float, exact lookup); record +0x87e = bDirty (reflected bool). UCOMISS sets ZF=0 only for ordered unequal operands, and only that case sets bDirty before the store.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* Record = NativeGetSettingsRecord144d03d50();
if (Record != nullptr)
{
    if (NewValue < Record->TouchLookAccelerationMultiplier || NewValue > Record->TouchLookAccelerationMultiplier)
    {
        Record->bDirty = true;
    }
    Record->TouchLookAccelerationMultiplier = NewValue;
}
