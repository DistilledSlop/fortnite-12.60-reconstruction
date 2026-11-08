// Original body 0x144d15530 (thunk 0x1452d01d0). Helper 0x144d03d50 = UFortSettingsContext::NativeGetSettingsRecord144d03d50 yields the settings record;
// the native body does nothing when it is null.
// Offset evidence: no inline record field access; the store is inside UFortClientSettingsRecord::SetCreativeTurboDelete at 0x141e3af40 (tail/direct call with RCX=record, DL=bInChecked).
// Job data: thunk 0x1452d01d0 and body 0x144d15530 are shared with OnCreativeTurboDeleteChanged (identical body).
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetCreativeTurboDelete(bInChecked);
}
