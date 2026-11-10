// CONFIDENCE: MEDIUM
// Original target 0x1419f1ed0. Header order: Mnemonic (FString*), Portal (UObject*).
// Helper calls in the decompile receive `this` in their first argument slot (FUN_1418c6640 and FUN_141ba8b20 read this).
if (Mnemonic == nullptr)
{
	return false;
}
if (PlotManager_0x828 == nullptr)	// FUN_1418c6640: returns this+0x828
{
	return false;
}
if (static_cast<int32>(Portal->ObjectFlags) < 2)
{
	return false;
}
// FUN_141ba8b20(this): the PlayerState from the GameState player array whose PlayerId (+0x336) matches this controller's id (+0x350).
auto* Owned = Helper_141ba8b20(this);
if (Owned == nullptr)
{
	return true;
}
auto* CallerState = PlayerState;
if (CallerState != nullptr && !CallerState->IsA(AFortPlayerState::StaticClass()))
{
	CallerState = nullptr;
}
return Helper_141ba8b20(this) == CallerState;
