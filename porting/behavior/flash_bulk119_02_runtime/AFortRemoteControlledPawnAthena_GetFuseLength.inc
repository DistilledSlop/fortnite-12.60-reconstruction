// CONFIDENCE: high
// Native returns 120.0f for a null set and otherwise reads the current value.
UFortRemoteControlPawnSet* PawnSet = RemoteControlPawnSet;
return PawnSet ? PawnSet->FuseLength.GetCurrentValue() : 120.0f;
