// Wrapper body at 0x144d18070 (thunk 0x1452d0b40); receiver this_delta 0, RCX = UFortSettingsContext.
// Offset evidence: no direct field access; the record is passed in RCX and bInChecked in DL to 0x141e3c600 (UFortClientSettingsRecord::SetShadowPlayHighlightsEnabled).
// Callee 0x144d03d50 = NativeGetSettingsRecord144d03d50 (declared in UFortSettingsContext.h); null check on the record.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* ClientRecord = NativeGetSettingsRecord144d03d50();
if (ClientRecord)
{
    ClientRecord->SetShadowPlayHighlightsEnabled(bInChecked);
}
