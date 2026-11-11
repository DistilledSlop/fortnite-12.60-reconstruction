// CONFIDENCE: MEDIUM. Builds a new FFortItemEntry for InItemDefinition with InCount and InLevel. The constructor clamps count and level to zero or above. The level is then passed through the item definition's level-adjust virtual (native vslot 0x2d8, unnamed). Other fields take the FFortItemEntry constructor defaults.
// Original address: 0x1429684e0 (exec). Native helpers: 0x141eea420 (wrapper), 0x1420237f0 (FFortItemEntry constructor), 0x142025400 (copy out), 0x1419844f0 (temporary destructor).
FFortItemEntry Entry;
if (InItemDefinition == nullptr)
{
    return Entry; // native dereferences the definition unconditionally; a null guard is added here
}
Entry = FFortItemEntry(InItemDefinition, FMath::Max(InCount, 0), FMath::Max(InLevel, 0));
Entry.Level = InItemDefinition->GetAdjustedItemLevel(Entry.Level);
return Entry;
