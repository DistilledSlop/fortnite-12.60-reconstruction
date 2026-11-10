// CONFIDENCE: LOW
// Original: 0x1423ad610 (virtual Implementation of ServerUpdateGameplayOptions). Callees not named here are described in the evidence file.
// Validate the requested gameplay options against the playlist and apply them to the pawn options or to the playlist.
if (UserOptionsKeys.Num() != UserOptionsValues.Num())
{
	UE_LOG(LogFort, Warning, TEXT("Gameplay Options: Mismatched key/value counts for target actor: %s"), *GetNameSafe(this));
	return;
}
if (!NativeCanPlayerEditGameplayOptions())
{
	return;
}
// Pawn-level gate: a locked entry or one in a non-editable state ignores the update.
if (UGameplayOptionsManager* Manager = NativeGetGameplayOptionsManager())
{
	if (UGameplayOptionsEntry* Entry = NativeFindGameplayOptionsEntry(Manager, GetPawn()))
	{
		if (Entry->NativeRecoveredByte_0x408 == 1 || (Entry->NativeRecoveredFlags_0x2d8 & 1))
		{
			return;
		}
		if (UGameplayOptionsState* State = NativeGetGameplayOptionsState(Entry))
		{
			const uint8 StateValue = State->NativeRecoveredByte_0x870;
			if (StateValue >= 1 && StateValue <= 5)
			{
				return;
			}
		}
	}
}
UWorld* World = GetWorld();
if (!World)
{
	return;
}
AGameStateBase* GameState = World->GetGameState();
UPawnOptions* PawnOptions = NativeGetPawnOptions(GetPawn());
FName PlaylistName = PawnOptions ? PawnOptions->PlaylistName : NAME_None;
UFortPlaylist* Playlist = nullptr;
if (PlaylistName == NAME_None && GameState)
{
	Playlist = GetCurrentPlaylistAthena(GameState);
	PlaylistName = Playlist ? Playlist->GetFName() : NAME_None;
}
if (PlaylistName == NAME_None)
{
	UE_LOG(LogFort, Warning, TEXT("Gameplay Options: Failed to find User Options for validation for target actor: %s"), *GetNameSafe(this));
	return;
}
TMap<FString, FString> Options;
for (int32 Index = 0; Index < UserOptionsKeys.Num(); ++Index)
{
	Options.Add(UserOptionsKeys[Index], UserOptionsValues[Index]);
}
TSet<FString> ValidOptions = NativeGetValidOptionsForPlaylist(PlaylistName, Options);
if (ValidOptions.Num() < Options.Num())
{
	for (const auto& Pair : Options)
	{
		if (!ValidOptions.Contains(Pair.Key))
		{
			UE_LOG(LogFort, Warning, TEXT("Gameplay Options: Received invalid user option (%s)/(%s) for actor: %s"), *Pair.Key, *Pair.Value, *GetNameSafe(this));
		}
	}
}
else if (!PawnOptions)
{
	if (!GameState || !(Playlist = GetCurrentPlaylistAthena(GameState)))
	{
		UE_LOG(LogFort, Warning, TEXT("Gameplay Options: Failed to find GameState or Playlist for actor: %s"), *GetNameSafe(this));
	}
	else
	{
		Playlist->NativeApplyOptions(Options);
		Playlist->NativeNotifyOptionsChanged();
		GameState->NativeNotifyOptionsChanged();
	}
}
else
{
	NativeRecordPlayerOptions(this, Options);
	PawnOptions->NativeApplyOptions(Options);
	NativeNotifyPawnOptionsOwner(PawnOptions);
}
