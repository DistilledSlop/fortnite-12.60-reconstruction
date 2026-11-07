// Original body 0x144cc5f40 (thunk 0x145244d90). Offset evidence: inlined hash probe over TMap Features (+0x1c0) keyed by Feature (DL); on a hit MOV byte [value+1],SIL stores ForcedState (R8 arg) and MOV byte [value+2],R9B stores Reason (R9 arg) into FFortUIFeatureStruct; on a miss nothing is written.
// CONFIDENCE: MEDIUM
if (FFortUIFeatureStruct* FeatureState = Features.Find(Feature))
{
    FeatureState->ForcedState = ForcedState;
    FeatureState->ForcedStateReason = Reason;
}
