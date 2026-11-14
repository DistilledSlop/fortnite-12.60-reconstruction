// CONFIDENCE: HIGH. Original142538a40; these three native tags use exact equality.
UWorld* World = GetWorld();
AFortGameState* State = World ? Cast<AFortGameState>(World->GetGameState()) : nullptr;
if (State && State->GameplayState == EFortGameplayState::EndOfZone)
{
    const FFortNativeGameplayTagsRuntime& Tags = FFortNativeGameplayTagsRuntime::Get();
    if (ActionTag == Tags.GetTagAtOriginalOffset(0x710) ||
        ActionTag == Tags.GetTagAtOriginalOffset(0x708) ||
        ActionTag == Tags.GetTagAtOriginalOffset(0x718))
    {
        if (AFortPlayerPawn* Pawn = GetPlayerPawn())
        {
            return Pawn->NativeCanPerformAction14230c260(ActionTag);
        }
    }
}
return AFortPlayerController::NativeCanPerformAction15f0(ActionTag);
