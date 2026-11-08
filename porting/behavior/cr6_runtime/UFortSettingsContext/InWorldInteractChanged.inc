// Original address: 0x144d12b60 (thunk 0x1452cf5f0)
// Offset evidence: record +0x46d = TouchInteractionMode (ETouchInteractMode, Enum byte, exact); record +0x87e = bDirty (bitfield, mov8). The stored byte is (bInChecked ^ 1) + 1 (XOR BL,1; INC BL), i.e. 1 when checked and 2 otherwise. The mode compare and both writes are skipped together when the value is unchanged (JZ to epilogue).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortClientSettingsRecord) > 0, "UFortClientSettingsRecord must be a complete type");
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    const uint8 Mode = bInChecked ? 1 : 2;
    if (static_cast<uint8>(SettingsRecord->TouchInteractionMode) != Mode)
    {
        SettingsRecord->TouchInteractionMode = static_cast<ETouchInteractMode>(Mode);
        SettingsRecord->bDirty = true;
    }
}
