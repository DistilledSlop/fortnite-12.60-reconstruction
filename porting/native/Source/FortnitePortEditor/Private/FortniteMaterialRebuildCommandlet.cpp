#include "FortniteMaterialRebuildCommandlet.h"

#include "AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "UObject/Package.h"

UFortniteMaterialRebuildCommandlet::UFortniteMaterialRebuildCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
}

int32 UFortniteMaterialRebuildCommandlet::Main(const FString& Params)
{
    FString MaterialPath;
    FString TexturePath;
    FString Blend;
    FParse::Value(*Params, TEXT("Texture="), TexturePath);
    FParse::Value(*Params, TEXT("Blend="), Blend);
    if (!FParse::Value(*Params, TEXT("Material="), MaterialPath) ||
        !FPackageName::IsValidLongPackageName(MaterialPath) ||
        !MaterialPath.StartsWith(TEXT("/Game/")) ||
        (Blend != TEXT("Translucent") && Blend != TEXT("Additive")))
    {
        UE_LOG(LogTemp, Error, TEXT("Use -Material=/Game/path/name -Blend=Translucent|Additive [-Texture=/Game/path/name.name]"));
        return 2;
    }

    UTexture2D* Texture = TexturePath.IsEmpty() ? nullptr : LoadObject<UTexture2D>(nullptr, *TexturePath);
    if (!TexturePath.IsEmpty() && !Texture)
    {
        UE_LOG(LogTemp, Error, TEXT("Texture did not load: %s"), *TexturePath);
        return 3;
    }

    UPackage* Package = CreatePackage(nullptr, *MaterialPath);
    UMaterial* Material = NewObject<UMaterial>(Package, *FPackageName::GetShortName(MaterialPath),
        RF_Public | RF_Standalone);
    Material->BlendMode = Blend == TEXT("Additive") ? BLEND_Additive : BLEND_Translucent;
    Material->SetShadingModel(MSM_Unlit);

    if (Texture)
    {
        UMaterialExpressionTextureSample* Sample = NewObject<UMaterialExpressionTextureSample>(Material);
        Sample->Texture = Texture;
        Sample->SamplerType = SAMPLERTYPE_Color;
        Material->Expressions.Add(Sample);
        Material->EmissiveColor.Connect(0, Sample); // RGB
        if (Material->BlendMode == BLEND_Translucent)
            Material->Opacity.Connect(4, Sample); // Alpha
    }
    else
    {
        UMaterialExpressionConstant3Vector* Color = NewObject<UMaterialExpressionConstant3Vector>(Material);
        Color->Constant = FLinearColor::White;
        Material->Expressions.Add(Color);
        Material->EmissiveColor.Connect(0, Color);
    }
    Material->PostEditChange();

    FAssetRegistryModule::AssetCreated(Material);
    Package->MarkPackageDirty();
    const FString Filename = FPackageName::LongPackageNameToFilename(
        MaterialPath, FPackageName::GetAssetPackageExtension());
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
    if (!UPackage::SavePackage(Package, Material, RF_Standalone, *Filename, GError))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not save material: %s"), *Filename);
        return 4;
    }

    UE_LOG(LogTemp, Display, TEXT("Saved reconstructed material: %s"), *Filename);
    return 0;
}
