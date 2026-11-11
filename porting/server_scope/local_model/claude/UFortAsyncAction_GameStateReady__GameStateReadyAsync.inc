// CONFIDENCE: MEDIUM. Factory for the async action that fires OnReady once the game state is available. Returns null when the world context has no world; otherwise creates the action, binds the world context, and activates it.
// Original addresses: exec 0x1427bca80; helper 0x14186ec90.
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);   // helper 0x144640410
if (!World)
{
	return nullptr;
}

UFortAsyncAction_GameStateReady* Action = NewObject<UFortAsyncAction_GameStateReady>();
Action->WorldContextObject = World;     // weak pointer at +0x8 of the base async action
Action->RegisterWithWorld(World);       // virtual 0x270 on the action (name proposed)
return Action;
