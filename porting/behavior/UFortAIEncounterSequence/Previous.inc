// CONFIDENCE: high
// Native target 0x1416a7a20.
const int32 PreviousIndex = static_cast<int32>(static_cast<uint32>(CurrentEncounterIndexInSequence) - 1u);
const int32 LastIndex = static_cast<int32>(static_cast<uint32>(GeneratedEncounterSequence.NumEncountersInSequence) - 1u);
CurrentEncounterIndexInSequence = PreviousIndex < 0 ? 0 : (PreviousIndex < LastIndex ? PreviousIndex : LastIndex);
return EFortEncounterSequenceResult::Success;
