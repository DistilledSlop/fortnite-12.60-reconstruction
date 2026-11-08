// Original body address: 0x14162bca0 (thunk 0x1427111c0)
// Offset evidence: Slots = reflected TArray at this+0x238 (Num +0x240); NoSlotAssignees = reflected TArray<AAIController*> at this+0x268 (Num +0x270).
// Helper NativeHelper141291580(slot) returning 1 or 2 (DEC AL; CMP AL,1; JA) marks the slot as assigned.
// Filter 0: assigned or listed in NoSlotAssignees; Filter 1: assigned; Filter 2: listed in NoSlotAssignees; any other value: false.
// CONFIDENCE: MEDIUM
const uint8 FilterValue = static_cast<uint8>(Filter);
bool bSlotAssigned = false;
const int32 SlotIndex = GetSlotIndexByOwner(AI);
if (SlotIndex >= 0 && SlotIndex < Slots.Num())
{
    UAIHotSpotSlot* const SlotObject = Slots[SlotIndex];
    if (SlotObject != nullptr)
    {
        const uint8 SlotState = NativeHelper141291580(reinterpret_cast<AAIHotSpot*>(SlotObject));
        bSlotAssigned = (SlotState == 1 || SlotState == 2);
    }
}
bool bInNoSlotAssignees = false;
for (int32 Index = 0; Index < NoSlotAssignees.Num(); ++Index)
{
    if (NoSlotAssignees[Index] == AI)
    {
        bInNoSlotAssignees = true;
        break;
    }
}
if (FilterValue == 0)
{
    return bSlotAssigned || bInNoSlotAssignees;
}
if (FilterValue == 1)
{
    return bSlotAssigned;
}
if (FilterValue == 2)
{
    return bInNoSlotAssignees;
}
return false;
