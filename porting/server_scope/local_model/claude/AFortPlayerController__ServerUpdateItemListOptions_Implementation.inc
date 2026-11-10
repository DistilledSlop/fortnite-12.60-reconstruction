// CONFIDENCE: MEDIUM
// Original: 0x1423add90 (virtual Implementation of ServerUpdateItemListOptions). Callees not named here are described in the evidence file.
// Store per-item options into an item list component. The caller must be allowed to edit it.
AFortGameStateAthena* GameState = nullptr;
if (UWorld* World = GetWorld())
{
	GameState = Cast<AFortGameStateAthena>(World->GameState);
}
UFortOptionsSettings* Settings = NativeGetItemOptionsSettings();
if (Settings->NativeRecoveredFlag_0xc9 && GameState && NativeCanEditItemOptions(GameState))
{
	if (!ItemContainerComponent)
	{
		UE_LOG(LogFort, Warning, TEXT("ITEM OPTIONS: Missing item list component. Does it not exist on the server?"));
	}
	else if (!NativeCanPlayerEditItemListComponent(ItemContainerComponent, this))
	{
		UE_LOG(LogFort, Warning, TEXT("ITEM OPTIONS: Player isn't allowed to edit item list component: %s"), *GetNameSafe(ItemContainerComponent));
	}
	else if (OptionsKeys.Num() == OptionsValues.Num())
	{
		TMap<FString, FString> Options;
		for (int32 Index = 0; Index < OptionsKeys.Num(); ++Index)
		{
			Options.Add(OptionsKeys[Index], OptionsValues[Index]);
		}
		ItemContainerComponent->NativeSetItemOptions(Options, ItemIndex);
	}
	else
	{
		UE_LOG(LogFort, Warning, TEXT("ITEM OPTIONS: Mismatched key/value counts for target component: %s"), *GetNameSafe(ItemContainerComponent));
	}
}
