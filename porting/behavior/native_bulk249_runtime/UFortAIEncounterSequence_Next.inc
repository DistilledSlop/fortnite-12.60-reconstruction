// CONFIDENCE: high
// Native target 0x1416a1e20.
const int32 NextIndex = static_cast<int32>(static_cast<uint32>(CurrentEncounterIndexInSequence) + 1u);
const int32 LastIndex = static_cast<int32>(static_cast<uint32>(GeneratedEncounterSequence.NumEncountersInSequence) - 1u);
CurrentEncounterIndexInSequence = NextIndex < 0 ? 0 : (NextIndex < LastIndex ? NextIndex : LastIndex);
return EFortEncounterSequenceResult::Success;
