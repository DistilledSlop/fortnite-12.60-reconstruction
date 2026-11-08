// Original body 0x144d158a0 (thunk 0x1452d06d0). Helper 0x144d03d50 = UFortSettingsContext::NativeGetSettingsRecord144d03d50 yields the settings record;
// the native body does nothing when it is null.
// Offset evidence: record+0x47e = bUseGamepadAutoRun (bool_bits.json mask 0x1, mov8); record+0x87e = bDirty (mask 0x1, mov8). asm: CMP BL,byte ptr [RAX + 0x47e]; JZ skip; MOV byte [RAX+0x47e],BL; MOV byte [RAX+0x87e],1.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord && SettingsRecord->bUseGamepadAutoRun != bInChecked)
{
    SettingsRecord->bUseGamepadAutoRun = bInChecked;
    SettingsRecord->bDirty = true;
}
