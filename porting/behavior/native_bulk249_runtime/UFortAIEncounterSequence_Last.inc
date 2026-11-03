// CONFIDENCE: high
// Native target 0x1416a15b0.
const int32 LastIndex = static_cast<int32>(static_cast<uint32>(GeneratedEncounterSequence.NumEncountersInSequence) - 1u);
CurrentEncounterIndexInSequence = LastIndex < 0 ? 0 : LastIndex;
return EFortEncounterSequenceResult::Success;
