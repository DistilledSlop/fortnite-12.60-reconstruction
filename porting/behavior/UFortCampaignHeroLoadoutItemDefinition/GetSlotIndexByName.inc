// Native 0x142519a40. CONFIDENCE: high
for (int32 Index = 0; Index < CrewSlots.Num(); ++Index)
{
    if (CrewSlots[Index].SlotName == SlotName)
    {
        return Index;
    }
}
return INDEX_NONE;
