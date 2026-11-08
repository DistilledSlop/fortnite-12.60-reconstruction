// Original body 0x144d14fb0 (thunk 0x1452cff90). Helper 0x144d03d50 = UFortSettingsContext::NativeGetSettingsRecord144d03d50 yields the settings record;
// the native body does nothing when it is null.
// Offset evidence: no inline record field access; the store is inside UFortClientSettingsRecord::SetAutoSortConsumablesToRight at 0x141e3a770 (tail/direct call with RCX=record, DL=bInChecked).
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetAutoSortConsumablesToRight(bInChecked);
}
