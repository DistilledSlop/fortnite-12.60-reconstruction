// CONFIDENCE: high
// 0x1420318c0, preserving the original virtual dispatch and append order.
TSoftObjectPtr<UStaticMesh> StaticMesh;
if (GetPreviewStaticMeshAsset(StaticMesh))
    Assets.Emplace(StaticMesh);
TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
if (GetPreviewSkeletalMeshAsset(SkeletalMesh))
    Assets.Emplace(SkeletalMesh);
TSoftClassPtr<AActor> Prefab;
if (GetPreviewPrefabAsset(Prefab))
    Assets.Emplace(Prefab.ToSoftObjectPath());
const TSoftObjectPtr<UTexture2D> Image = GetLargePreviewImage();
if (!Image.IsNull())
    Assets.Emplace(Image);
