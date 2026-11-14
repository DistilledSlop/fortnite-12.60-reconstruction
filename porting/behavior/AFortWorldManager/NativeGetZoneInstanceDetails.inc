// CONFIDENCE: HIGH
// Original 0x141f886f0. Returned storage belongs to the record, first queued
// action, or a process-wide default, respectively; do not return a copy.
if (CurrentWorldRecord)
    return &CurrentWorldRecord->ZoneInstanceInfo;
if (NativeWorldRecordActions_0x6c0.Num() > 0)
{
    FFortZoneInstanceDetails& First = NativeWorldRecordActions_0x6c0[0].ZoneInstanceInfo;
    if (!First.ZoneThemeClass.IsNull() && (!First.WorldId.IsEmpty() || !First.TheaterId.IsEmpty()))
        return &First;
}
static FFortZoneInstanceDetails EmptyDetails;
return &EmptyDetails;
