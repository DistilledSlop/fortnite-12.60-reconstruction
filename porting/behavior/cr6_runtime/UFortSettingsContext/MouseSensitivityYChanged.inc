// Original address: 0x144d14d60 (thunk 0x1452cfa10)
// Offset evidence: this+0x30 is below UFortSettingsContext's lowest own offset (0x3c), so it is raw inherited storage (float). Record +0x310 = MouseSensitivityY (Float, exact); record +0x87e = bDirty (bitfield, mov8). Same constants as the X handler: 1e-08f at [0x1459c03a0] and the sign-clear mask at [0x1459a3d00].
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    uint8* Raw = reinterpret_cast<uint8*>(this);
    const float Scaled = NewValue * *reinterpret_cast<const float*>(Raw + 0x30);
    if (FMath::Abs(Scaled - SettingsRecord->MouseSensitivityY) > 1e-08f)
    {
        SettingsRecord->MouseSensitivityY = Scaled;
        SettingsRecord->bDirty = true;
    }
}
