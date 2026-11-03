// CONFIDENCE: high. Native target 0x142312640.
++EmoteCount;
if (UWorld* World = GetWorld())
{
    LastEmoteTime = World->GetTimeSeconds();
}
LastEmoteItemDef = MontageItemDef;
OnPawnStartedEmote.Broadcast(MontageItemDef);
