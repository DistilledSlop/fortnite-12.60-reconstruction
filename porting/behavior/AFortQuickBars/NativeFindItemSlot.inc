// CONFIDENCE: high
// Original 0x14204f0f0; assigned source helper name.
UFortItem* SlotItem = Item;
if (!SlotItem) return false;
const int32 Step = bReverse ? -1 : 1;
const uint8 Size = SlotItem->GetNumberOfSlotsTaken();
TArray<FGuid> Allowed = MoveGuids;
if (bAllowOwnGuid) Allowed.Add(SlotItem->NativeGetItemGuid());
int32 Index = StartSlot;
do
{
    if (NativeCanFitItemInSlots(Item, Bar, Index, Allowed, false))
    {
        FoundSlot = Index;
        return true;
    }
    Index += Step;
} while (Index >= 0 && Index <= Bar.Slots.Num() - int32(Size));
return false;
