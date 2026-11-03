// CONFIDENCE: HIGH
if (!GEngine || !GEngine->AssetManager)
{
    return 2;
}
return NativeGetFortAssetManager().NativeCurrentGameSubMode;
