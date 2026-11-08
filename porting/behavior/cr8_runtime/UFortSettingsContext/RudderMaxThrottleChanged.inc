// Wrapper body at 0x144d1d460 (thunk 0x1452d1640); receiver this_delta 0, RCX = UFortSettingsContext.
// Offset evidence: no direct field access; the record is passed in RCX and NewValue in XMM1 to 0x141e3c570 (UFortClientSettingsRecord::SetRudderMaxThrottle).
// Callee 0x144d03d50 = NativeGetSettingsRecord144d03d50 (declared in UFortSettingsContext.h); null check on the record.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* ClientRecord = NativeGetSettingsRecord144d03d50();
if (ClientRecord)
{
    ClientRecord->SetRudderMaxThrottle(NewValue);
}
