// CONFIDENCE: high
// 0x1420587e0 snapshots the entry array before constructing the result.
const TArray<UFortAlterationItemDefinition*> Alterations = ItemEntry.AlterationInstances;
TArray<FFortAlterationSlotStatus> Result;
Result.Reserve(Alterations.Num());
for (UFortAlterationItemDefinition* Alteration : Alterations)
{
    FFortAlterationSlotStatus& Slot = Result.AddDefaulted_GetRef();
    Slot.Alteration = Alteration;
    Slot.MinRequiredLevel = 0;
    Slot.MinHostItemRarity = EFortRarity::Common;
}
return Result;
