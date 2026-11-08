// Original body 0x144d157b0 (thunk 0x1452d0400). Helper 0x144d03d50 = UFortSettingsContext::NativeGetSettingsRecord144d03d50 yields the settings record;
// the native body does nothing when it is null.
// Offset evidence: record+0x47c = bUseGamepadEditModeAimAssist (bool_bits.json mask 0x1, mov8); record+0x87e = bDirty (mask 0x1, mov8). asm: CMP BL,byte ptr [RAX + 0x47c]; JZ skip; MOV byte [RAX+0x47c],BL; MOV byte [RAX+0x87e],1.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord && SettingsRecord->bUseGamepadEditModeAimAssist != bInChecked)
{
    SettingsRecord->bUseGamepadEditModeAimAssist = bInChecked;
    SettingsRecord->bDirty = true;
}
