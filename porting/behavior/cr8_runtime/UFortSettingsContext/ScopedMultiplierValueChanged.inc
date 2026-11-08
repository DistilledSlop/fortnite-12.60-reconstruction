// Wrapper body at 0x144d1d4b0 (thunk 0x1452d1740); receiver this_delta 0, RCX = UFortSettingsContext.
// Offset evidence: UFortClientSettingsRecord +0x2e8 = ScopedSlow (porting/generated/reflection/bool_bits.json or reflected float); +0x87e = bDirty (bool_bits.json, mask 1, mov8).
// Callee 0x144d03d50 = NativeGetSettingsRecord144d03d50 (declared in UFortSettingsContext.h); UCOMISS + JZ skip: JZ also skips on unordered (NaN), so the store runs only for ordered inequality.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* ClientRecord = NativeGetSettingsRecord144d03d50();
if (ClientRecord && (NewValue < ClientRecord->ScopedSlow || NewValue > ClientRecord->ScopedSlow))
{
    ClientRecord->bDirty = true;
    ClientRecord->ScopedSlow = NewValue;
}
