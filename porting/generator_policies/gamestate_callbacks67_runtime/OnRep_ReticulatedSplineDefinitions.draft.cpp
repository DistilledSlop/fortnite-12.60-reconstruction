// Exec 0x142920630. Behavioral draft; parsing, record and manager APIs unresolved.
void AFortGameStateAthena::OnRep_ReticulatedSplineDefinitions()
{
    if (ReticulatedSplineDefinitions.IsEmpty()) {
        UnresolvedLogNoSplines();
        return;
    }
    TArray<UnresolvedSplineRecord> Records;
    for (const FString& Definition : ReticulatedSplineDefinitions) {
        // 0x142bae420 split, delimiter at 0x145a95870, culls empty tokens.
        TArray<FString> Parts = UnresolvedSplit(Definition);
        if (Parts.Num() < 2) { UnresolvedLogBadSpline(); continue; }
        UnresolvedSplineRecord Record{};
        if (!UnresolvedParseGuid_142ca0820(Parts[0], Record)) {
            UnresolvedLogBadGuid(); continue;
        }
        // 0x142c716d0 supplies bytes from Parts[1]; exact encoding not established.
        Record.Bytes = UnresolvedDecode_142c716d0(Parts[1]);
        if (Record.Bytes.Num() != 32 || !UnresolvedReticulate_142c6b540(Record)) {
            UnresolvedLogReticulationFailure(); continue;
        }
        Records.Add(Record);
    }
    if (Records.Num()) {
        UnresolvedPrepareMerge_141d5ff50();
        UnresolvedLogMergeCount(Records.Num());
        auto* Manager = UnresolvedGetManager_141d460e0();
        UnresolvedMerge_141d55500(Manager, Records);
    } else UnresolvedLogNoSuccessfulSplines();
}
