// CONFIDENCE: high; captured shared paths and action field selection, see native-evidence.json.
UFortGameData* Data = UFortAssetManager::NativeGetFortAssetManager().NativeGetCommonGameData();
if (Data->FortInputData.IsNull())
    return nullptr;
UFortInputData* Result = Data->FortInputData.Get();
if (!Result)
    Result = Cast<UFortInputData>(FortLoadDelayedObject(Data->FortInputData.ToSoftObjectPath(), true));
FFortAssets::Get().TrackAsset(Result);
return Result;
