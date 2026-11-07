// Behaviour snippet: AFortPlayerStateZone::GetNumOfAccumulatedItem (body only).
// Original address: 0x1424182d0 (thunk 0x142a94d90)
// Offset evidence: this+0xcc8 = AccumulatedItems (reflected TArray<FAccumulatedItemEntry>, Num at +0xcd0); entry is 16 bytes: ItemDefinition at +0 (compared to the argument), Quantity int32 at +8 (summed).
// CONFIDENCE: HIGH
int32 Total = 0;
for (int32 i = 0; i < AccumulatedItems.Num(); ++i)
{
    if (AccumulatedItems[i].ItemDefinition == ItemDefinition)
    {
        Total += AccumulatedItems[i].Quantity;
    }
}
return Total;
