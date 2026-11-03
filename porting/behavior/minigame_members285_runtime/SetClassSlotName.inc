// Native 0x1420a3d30. CONFIDENCE: high
if (ClassSlotIndex == 0xff) return;
FMinigameClassSlot* Slot = ClassSlotArray.FindByPredicate([ClassSlotIndex](const FMinigameClassSlot& Entry) { return Entry.ClassSlotIndex == ClassSlotIndex; });
if (!Slot)
{
    Slot = &ClassSlotArray.AddDefaulted_GetRef();
    Slot->ClassSlotIndex = ClassSlotIndex;
}
Slot->ClassName = ClassName;
