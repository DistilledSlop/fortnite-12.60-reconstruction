// CONFIDENCE: HIGH
// Original: exec thunk 0x142a2ab30; helper 0x14202ecb0 (index lookup); copy 0x142025400 (FFortItemEntry assignment).
// The empty fallback is a function-local static FFortItemEntry default-constructed by 0x142023c10.
static const FFortItemEntry EmptyItemEntry{};

if (MultiItemPickupEntries.Num() == 0)
{
    if (ItemIndex == 0)
    {
        return PrimaryPickupItemEntry; // +0x290
    }
    return EmptyItemEntry;
}

if (ItemIndex >= 0 && ItemIndex < MultiItemPickupEntries.Num())
{
    return MultiItemPickupEntries[ItemIndex]; // +0x3e0 data, stride 0x150
}
return EmptyItemEntry;
