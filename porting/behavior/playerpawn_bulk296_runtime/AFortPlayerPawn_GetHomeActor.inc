// CONFIDENCE: high; recovered native control flow, see native-evidence.json.
const AFortPlayerState* FortPlayerState = GetPlayerState<AFortPlayerState>();
return FortPlayerState ? FortPlayerState->HomeActor : nullptr;
