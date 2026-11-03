// CONFIDENCE: HIGH
// Original address: 0x141ed7750.
if (GEngine)
{
    if (const UFortAssetManager* Manager = Cast<UFortAssetManager>(GEngine->AssetManager))
    {
        return Manager->NativeGetStartupPhase();
    }
}
return 0;
