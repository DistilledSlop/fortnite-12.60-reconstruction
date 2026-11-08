// Original address: 0x144d14d00 (thunk 0x1452cf990)
// Offset evidence: this+0x30 is below UFortSettingsContext's lowest own offset (0x3c), so it is raw inherited storage (float). Record +0x30c = MouseSensitivityX (Float, exact); record +0x87e = bDirty (bitfield, mov8). Constants: [0x1459c03a0] is 1e-08f in .rdata (checked in the PE image); [0x1459a3d00] is the sign-clear mask, so FMath::Abs matches ANDPS.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    uint8* Raw = reinterpret_cast<uint8*>(this);
    const float Scaled = NewValue * *reinterpret_cast<const float*>(Raw + 0x30);
    if (FMath::Abs(Scaled - SettingsRecord->MouseSensitivityX) > 1e-08f)
    {
        SettingsRecord->MouseSensitivityX = Scaled;
        SettingsRecord->bDirty = true;
    }
}
