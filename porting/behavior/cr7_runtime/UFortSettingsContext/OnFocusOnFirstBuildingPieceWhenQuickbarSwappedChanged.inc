// Original body 0x144d15810 (thunk 0x1452d0520). Helper 0x144d03d50 = UFortSettingsContext::NativeGetSettingsRecord144d03d50 yields the settings record;
// the native body does nothing when it is null.
// Offset evidence: record+0x480 = bNewFocusOnFirstBuildingPieceWhenQuickbarSwapped (bool_bits.json mask 0x1, mov8); record+0x87e = bDirty (mask 0x1, mov8). asm: CMP BL,byte ptr [RAX + 0x480]; JZ skip; MOV byte [RAX+0x480],BL; MOV byte [RAX+0x87e],1.
// asm writes +0x480 (bNewFocus...), not bFocus... at +0x47f (asm authoritative; header name kept as asm offset).
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord && SettingsRecord->bNewFocusOnFirstBuildingPieceWhenQuickbarSwapped != bInChecked)
{
    SettingsRecord->bNewFocusOnFirstBuildingPieceWhenQuickbarSwapped = bInChecked;
    SettingsRecord->bDirty = true;
}
