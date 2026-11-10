// CONFIDENCE: MEDIUM. Rarity override read from the collector's data-table row (ItemCollectorHandle). Returns false when there is no row or the row carries no override.
// Original address: 0x1427474c0 (exec). Real body: virtual slot 0xa58 -> 0x141bdec10 (named GetOverrideRarity in virtual_slots).
const FItemCollectorDataRow* Row = ItemCollectorHandle.GetRow<FItemCollectorDataRow>(TEXT("ABuildingItemCollectorActor::GetOverrideRarity"));
if (Row == nullptr || Row->RarityOverrideCount == 0)   // int at row+0x1c; reflected name not recovered
{
    return false;
}
Rarity = Row->Rarity;                                  // byte at row+0x10 (EFortRarity)
return true;
