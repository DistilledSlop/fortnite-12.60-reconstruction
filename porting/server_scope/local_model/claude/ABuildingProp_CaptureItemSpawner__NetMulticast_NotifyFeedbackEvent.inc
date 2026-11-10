// CONFIDENCE: LOW. Multicast capture feedback: build the feedback context for the zone game state, queue the item-name entry when feedback is enabled, notify the zone, then refresh feedback visuals.
// Original address: 0x14274b4f0 (exec). Real body: virtual slot 0xec0 -> 0x141c10bc0 (named NetMulticast_NotifyFeedbackEvent in virtual_slots).
AFortGameStateZone* Zone = nullptr;
if (UWorld* World = GetWorld())
{
    if (AFortGameStateZone* GameState = Cast<AFortGameStateZone>(World->GameState))   // native 0x142927a60 (AFortGameStateZone::StaticClass)
    {
        Zone = GameState;
    }
}
FFeedbackContext Context = MakeFeedbackContext(ItemEntry.Source, this, Zone);       // native 0x141fca6f0
if (bFeedbackEnabled)                                                                 // bool at +0xb58
{
    UObject* Instigator = GetFeedbackInstigator(EventParams);                       // native 0x141f88f00
    FFeedbackEntry Entry;
    Entry.Key = TEXT("ItemName");
    Entry.Value = ItemEntry.ItemName;
    Entry.Hash = FCrc::StrCrc32(ItemEntry.ItemName);                               // native 0x1407901a0
    FeedbackEntries.Add(Entry);                                                     // native 0x1407babb0 (sparse hash insert, 0x48-byte records)
    SendFeedbackEvent(this, Zone, Context, FeedbackEntries);                        // native 0x141fcb160
    FeedbackEntries.Remove(Entry);
}
RefreshFeedbackState(ItemEntry, Context.NumEntries == 0);                          // native 0x14274a140 (iStack_f0 is the context entry count)
