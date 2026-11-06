// CONFIDENCE: high; original source recovery, build/runtime pending.
UWorld* World = GEngine->GetWorldFromContextObject(this, EGetWorldErrorMode::ReturnNull);
AFortGameState* GameState = World ? Cast<AFortGameState>(World->GetGameState()) : nullptr;
AFortWindManager* WindManager = GameState ? GameState->WindManager : nullptr;
if (WindManager)
{
    const FVector Direction = FVector(1.0f, 0.0f, 0.0f).RotateAngleAxis(
        InitialGlobalWind.WindHeading, FVector(0.0f, 0.0f, 1.0f));
    WindManager->NativeSetGlobalWind(InitialGlobalWind.WindIntensity, Direction);
}
