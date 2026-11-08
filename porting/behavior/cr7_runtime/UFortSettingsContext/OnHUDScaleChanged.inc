// Original body 0x144d158d0 (thunk 0x1452d0760). Helper 0x144d03d50 = UFortSettingsContext::NativeGetSettingsRecord144d03d50 yields the settings record;
// the native body does nothing when it is null.
// Offset evidence: no inline record field access; the store is inside UFortClientSettingsRecord::SetHUDScale at 0x141e3b750 (tail/direct call with RCX=record, XMM1=bInHUDScale).
// CONFIDENCE: HIGH
UFortClientSettingsRecord* SettingsRecord = NativeGetSettingsRecord144d03d50();
if (SettingsRecord)
{
    SettingsRecord->SetHUDScale(bInHUDScale);
}
