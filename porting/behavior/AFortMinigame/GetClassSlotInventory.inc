// Native 0x14208f120. CONFIDENCE: high
if (ClassSlotIndex != 0xff)
{
    for (const FMinigameClassSlot& Slot : ClassSlotArray)
    {
        if (Slot.ClassSlotIndex == ClassSlotIndex)
        {
            InventoryItems = Slot.InventoryItems;
            return true;
        }
    }
}
return false;
