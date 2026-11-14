// CONFIDENCE: HIGH. Original1415e2820 specialized to GameData soft field dc0.
if (GameplayTagData.IsNull()) return nullptr;
UFortGameplayTagData* Data = GameplayTagData.Get();
if (!Data)
{
    Data = Cast<UFortGameplayTagData>(FortLoadDelayedObject(
        GameplayTagData.ToSoftObjectPath(), bLogLoad));
}
FFortAssets::Get().TrackAsset(Data);
return Data;
