// Wrapper body at 0x144d20900 (thunk 0x1452d2510); receiver this_delta 0, RCX = UFortSettingsContext.
// Offset evidence: UFortClientSettingsRecord +0x46b = bSprintCancelsReload (porting/generated/reflection/bool_bits.json or reflected float); +0x87e = bDirty (bool_bits.json, mask 1, mov8).
// Callee 0x144d03d50 = NativeGetSettingsRecord144d03d50 (declared in UFortSettingsContext.h); CMP byte + JZ skip: stores run only when the byte differs from the argument.
// CONFIDENCE: HIGH
UFortClientSettingsRecord* ClientRecord = NativeGetSettingsRecord144d03d50();
if (ClientRecord && ClientRecord->bSprintCancelsReload != bInChecked)
{
    ClientRecord->bSprintCancelsReload = bInChecked;
    ClientRecord->bDirty = true;
}
