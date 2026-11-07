// Original body address: 0x1450b68c0 (thunk 0x145214660)
// Offset evidence: dword [RCX+0x290] and dword [RCX+0x294] = SlotRowName (reflected FName at 0x290, width 8); both halves compared with SlotId (R9 = arg3).
// On full match qword [RCX+0x358] = 0 = SlottedItemRepresentation (reflected Object at 0x358). UnslottedItem and OldItem are not read.
// CONFIDENCE: HIGH
if (SlotRowName == SlotId)
{
    SlottedItemRepresentation = nullptr;
}
