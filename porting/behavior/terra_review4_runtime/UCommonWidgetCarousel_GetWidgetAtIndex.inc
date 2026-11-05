// CONFIDENCE: HIGH
// Reviewed against captured native wrapper; build and runtime pending.
if (!Slots.IsValidIndex(Index))
{
    return nullptr;
}
return Slots[Index]->Content;
