// Original address: 0x144d14a40 (thunk 0x1452cf700)
// Offset evidence: record byte +0x338 bit 0 = bMotionControlsEnabled (bool bitfield, bool_bits.json; the only declared bit in that byte). record +0x87e = bDirty (bitfield, mov8). bDirty is written only when the stored value changes; the bit store is unconditional on the non-null path.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    if (SettingsRecord->bMotionControlsEnabled != NewValue)
    {
        SettingsRecord->bDirty = true;
    }
    SettingsRecord->bMotionControlsEnabled = NewValue;
}
