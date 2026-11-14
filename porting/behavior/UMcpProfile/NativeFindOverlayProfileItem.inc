// CONFIDENCE: HIGH. Complete original body; broader lifecycle remains open.
// 0x1410fde50, semantic body. Both arguments are from the same real backend.
// A present null override is returned as-is; only an absent key reaches base.
if (const FMcpProfileItemPtr* Found = Overlay.OverridesByItemId.Find(ItemId))
{
    return *Found;
}
return NativeFindBaseProfileItem(ItemsById, ItemId);
