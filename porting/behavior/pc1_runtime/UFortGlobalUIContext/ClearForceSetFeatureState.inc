// Original body 0x144cc28c0 (thunk 0x145244840). Offset evidence: inlined hash probe over TMap Features (+0x1c0, TMap<EFortUIFeature, FFortUIFeatureStruct>) keyed by Feature; on a hit the word store MOV word [value+1],0x0803 writes ForcedState=3 (+1) and ForcedStateReason=8 (+2) of FFortUIFeatureStruct; on a miss nothing is written.
// CONFIDENCE: MEDIUM
if (FFortUIFeatureStruct* FeatureState = Features.Find(Feature))
{
    FeatureState->ForcedState = static_cast<EFortUIFeatureState>(3);
    FeatureState->ForcedStateReason = static_cast<EFortUIFeatureStateReason>(8);
}
