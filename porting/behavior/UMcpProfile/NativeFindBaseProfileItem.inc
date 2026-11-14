// CONFIDENCE: HIGH. Complete original body; broader lifecycle remains open.
// 0x1410fdd30, semantic body. ItemsById is the actual backend's base index.
if (const FMcpProfileItemPtr* Found = ItemsById.Find(ItemId))
{
    return *Found;
}
return FMcpProfileItemPtr();
