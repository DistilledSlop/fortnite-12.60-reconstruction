// Original address: 0x144d00850 (thunk 0x1452cd8a0)
// Offset evidence: record +0x2f0 = GamepadScopedSlow (Float, exact); record +0x87e = bDirty (bitfield, mov8). UCOMISS sets ZF for equal and unordered, so bDirty is written only for ordered unequal values; the field store at 0x144d00876 runs unconditionally on the non-null path.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    if (NewValue < SettingsRecord->GamepadScopedSlow || NewValue > SettingsRecord->GamepadScopedSlow)
    {
        SettingsRecord->bDirty = true;
    }
    SettingsRecord->GamepadScopedSlow = NewValue;
}
