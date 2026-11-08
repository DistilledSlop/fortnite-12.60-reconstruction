// Original address: 0x144d0f7b0 (thunk 0x1452cf3b0)
// Offset evidence: record +0x33c = GyroSensitivity (Float, exact); record +0x87e = bDirty (bitfield, mov8). bDirty is written only for ordered unequal values (JZ over the flag write); the field store is unconditional on the non-null path.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    if (NewValue < SettingsRecord->GyroSensitivity || NewValue > SettingsRecord->GyroSensitivity)
    {
        SettingsRecord->bDirty = true;
    }
    SettingsRecord->GyroSensitivity = NewValue;
}
