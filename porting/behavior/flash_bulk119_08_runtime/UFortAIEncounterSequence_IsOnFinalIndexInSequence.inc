// CONFIDENCE: high
// Native body at 0x1416a1280 compares CurrentEncounterIndexInSequence with GeneratedEncounterSequence.NumEncountersInSequence - 1.
return static_cast<uint32>(CurrentEncounterIndexInSequence) == (static_cast<uint32>(GeneratedEncounterSequence.NumEncountersInSequence) - 1u);
