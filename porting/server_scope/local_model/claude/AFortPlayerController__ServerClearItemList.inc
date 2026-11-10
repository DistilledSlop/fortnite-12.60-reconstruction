// CONFIDENCE: MEDIUM. Original exec 0x142a4a650 (RPC thunk, one object parameter: the item container component).
// Validate is virtual slot 0x13d0 -> Stub_ReturnTrue (0x1406801d0), so no validation runs.
// Implementation is virtual slot 0x13d8 -> 0x1423aa7f0 (55 lines), inlined below.
// Unresolved: the engine flag at +0xc9 (engine getter 0x141ed2350 is not ported), the item-options gate (0x14199e3c0 -> 0x141a4a6f0, see deps), the container's virtual slot 0x3e8 (ClearItemList, name unresolved) and the name formatter 0x142ce9f90.
AGameStateBase* GameState = nullptr;
if (UWorld* World = GetWorld())
{
	GameState = World->GameState;
	if (GameState && !GameState->IsA(AFortGameStateAthena::StaticClass()))
	{
		GameState = nullptr;
	}
}
if (GetFortEngine()->ItemOptionsEnabled_0xc9 != 0 && GameState != nullptr && ItemOptionsGate(this))
{
	if (ItemContainerComponent == nullptr)
	{
		UE_LOG(LogFortItems, Warning, TEXT("ITEM OPTIONS: Missing item list component. Does it not exist on the server?"));
		return;
	}
	if (!CanPlayerEditItemListComponent(ItemContainerComponent, this))
	{
		UE_LOG(LogFortItems, Warning, TEXT("ITEM OPTIONS: Player isn't allowed to edit item list component: %s"), *ItemContainerComponent->GetNameForLog_0x142ce9f90());
		return;
	}
	ItemContainerComponent->ClearItemList_Slot0x3e8();
}
