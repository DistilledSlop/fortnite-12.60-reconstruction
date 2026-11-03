// CONFIDENCE: high. Native target 0x142312c40.
if (LastEmoteItemDef == MontageItemDef || LastEmoteItemDef == nullptr)
{
    LastEmoteTime = -1.0f;
    EmoteCount = 0;
    LastEmoteItemDef = nullptr;
}
if (UWorld* World = GetWorld())
{
    LastEmoteEndTime = World->GetTimeSeconds();
    bFireBlockedByEmoteCooldown = false;
}
