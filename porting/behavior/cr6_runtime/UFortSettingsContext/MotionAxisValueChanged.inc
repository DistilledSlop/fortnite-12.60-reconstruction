// Original address: 0x144d14a00 (thunk 0x1452cf680)
// Offset evidence: record +0x339 = MotionYawAxis (EFortMotionYawAxis, Enum byte, exact); record +0x87e = bDirty (bitfield, mov8). The stored byte is 1 when NewValue is nonzero, else 0 (TEST EBX,EBX). bDirty is written only when the stored byte differs; the axis store itself is unconditional on the non-null path.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    const uint8 Axis = (NewValue != 0) ? 1 : 0;
    if (static_cast<uint8>(SettingsRecord->MotionYawAxis) != Axis)
    {
        SettingsRecord->bDirty = true;
    }
    SettingsRecord->MotionYawAxis = static_cast<EFortMotionYawAxis>(Axis);
}
