// Original body 0x144d15780 (thunk 0x1452d0370). Helper 0x144d03d50 = UFortSettingsContext::NativeGetSettingsRecord144d03d50 yields the settings record;
// the native body does nothing when it is null.
// Offset evidence: record+0x474 = bEditConfirmOnReleaseEnabled (bool_bits.json mask 0x1, mov8); record+0x87e = bDirty (mask 0x1, mov8). asm: CMP byte ptr [RAX + 0x474],BL; JZ skip; MOV byte [RAX+0x474],BL; MOV byte [RAX+0x87e],1.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord && SettingsRecord->bEditConfirmOnReleaseEnabled != bInChecked)
{
    SettingsRecord->bEditConfirmOnReleaseEnabled = bInChecked;
    SettingsRecord->bDirty = true;
}
