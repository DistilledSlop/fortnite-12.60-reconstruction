// CONFIDENCE: MEDIUM. Impl at 0x1423ac490 is fully read. Flags are at this+0x6f8 and this+0x6f9 (proposed names).
const bool bWasFinished = bClientHasFinishedLoading && bClientReadyFlag;
bClientHasFinishedLoading = bInHasFinishedLoading;
if (bClientReadyFlag && bInHasFinishedLoading && !bWasFinished)
{
    OnClientFinishedLoading();   // virtual slot 0x1588 (FUN_14239a9e0, not read)
}
