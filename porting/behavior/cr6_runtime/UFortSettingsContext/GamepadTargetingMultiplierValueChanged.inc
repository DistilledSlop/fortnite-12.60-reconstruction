// Original address: 0x144d00890 (thunk 0x1452cd920)
// Offset evidence: record +0x2ec = GamepadTargetingSlow (Float, exact); record +0x87e = bDirty (bitfield, mov8). Same flag/store pattern as the scoped multiplier: bDirty only for ordered unequal values, field store unconditional on the non-null path.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    if (NewValue < SettingsRecord->GamepadTargetingSlow || NewValue > SettingsRecord->GamepadTargetingSlow)
    {
        SettingsRecord->bDirty = true;
    }
    SettingsRecord->GamepadTargetingSlow = NewValue;
}
