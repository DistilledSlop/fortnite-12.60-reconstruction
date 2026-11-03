// CONFIDENCE: HIGH
if (Overlay.DirtyGroups.Remove(Key) == 0)
{
    if (const FMcpProfileItemArray* Cached = Overlay.CachedGroups.Find(Key))
    {
        return Cached;
    }
    return NativeFindBaseProfileGroup(BaseGroups, Key);
}

FMcpProfileItemArray Combined;
if (NativeRecomputeOverlayProfileGroup(BaseGroups, Overlay.OverridesByItemId, Key, Combined) == 0)
{
    Overlay.CachedGroups.Remove(Key);
    return NativeFindBaseProfileGroup(BaseGroups, Key);
}

FMcpProfileItemArray& Cached = Overlay.CachedGroups.FindOrAdd(Key);
Cached = Combined; // independent owning shared references, including empty results
return &Cached;
