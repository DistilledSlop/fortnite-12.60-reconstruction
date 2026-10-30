#include "FortniteMaterialRebuildCommandlet.h"

#include "AssetRegistryModule.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "HAL/FileManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionTextureSample.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
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
    if (FParse::Param(*Params, TEXT("AllApolloTiles")))
    {
        int32 Created = 0;
        int32 Existing = 0;
        int32 Failed = 0;
        for (TCHAR Row = TEXT('A'); Row <= TEXT('H'); ++Row)
        {
            for (int32 Column = 1; Column <= 8; ++Column)
            {
                const FString Tile = FString::Printf(TEXT("%c%d"), Row, Column);
                if (Tile == TEXT("A7") || Tile == TEXT("A8") || Tile == TEXT("B8"))
                    continue;
                const FString Base = TEXT("/Game/Athena/Apollo/Maps/Landscape/Converted/");
                const FString MaterialPath = Base + TEXT("Materials/M_Terrain_") + Tile;
                const FString Filename = FPackageName::LongPackageNameToFilename(
                    MaterialPath, FPackageName::GetAssetPackageExtension());
                if (IFileManager::Get().FileExists(*Filename))
                {
                    Existing++;
                    continue;
                }
                const FString ChildParams = FString::Printf(
                    TEXT("-Material=%s -Blend=Opaque -Texture=%sTextures/Terrain_%s.Terrain_%s -UVChannel=1 -Mesh=%sLandscapeStreamingProxy_%s.LandscapeStreamingProxy_%s"),
                    *MaterialPath, *Base, *Tile, *Tile, *Base, *Tile, *Tile);
                if (Main(ChildParams) == 0)
                    Created++;
                else
                    Failed++;
            }
        }
        UE_LOG(LogTemp, Display, TEXT("Apollo terrain material batch: %d created, %d existing, %d failed"),
            Created, Existing, Failed);
        return Created + Existing == 61 && Failed == 0 ? 0 : 7;
    }

    FString MaterialPath;
    FString TexturePath;
    FString MeshPath;
    FString Blend;
    int32 UVChannel = 0;
    FParse::Value(*Params, TEXT("Texture="), TexturePath);
    FParse::Value(*Params, TEXT("Mesh="), MeshPath);
    FParse::Value(*Params, TEXT("Blend="), Blend);
    FParse::Value(*Params, TEXT("UVChannel="), UVChannel);
    if (!FParse::Value(*Params, TEXT("Material="), MaterialPath) ||
        !FPackageName::IsValidLongPackageName(MaterialPath) ||
        !MaterialPath.StartsWith(TEXT("/Game/")) ||
        (Blend != TEXT("Translucent") && Blend != TEXT("Additive") && Blend != TEXT("Opaque")) ||
        UVChannel < 0 || UVChannel > 7 || (!MeshPath.IsEmpty() && Blend != TEXT("Opaque")))
    {
        UE_LOG(LogTemp, Error, TEXT("Use -Material=/Game/path/name -Blend=Translucent|Additive|Opaque [-Texture=/Game/path/name.name] [-UVChannel=0..7] [-Mesh=/Game/path/name.name for Opaque]"));
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
    Material->BlendMode = Blend == TEXT("Additive") ? BLEND_Additive :
        Blend == TEXT("Opaque") ? BLEND_Opaque : BLEND_Translucent;
    Material->SetShadingModel(Blend == TEXT("Opaque") ? MSM_DefaultLit : MSM_Unlit);

    if (Texture)
    {
        UMaterialExpressionTextureSample* Sample = NewObject<UMaterialExpressionTextureSample>(Material);
        Sample->Texture = Texture;
        Sample->SamplerType = SAMPLERTYPE_Color;
        Material->Expressions.Add(Sample);
        if (UVChannel > 0)
        {
            UMaterialExpressionTextureCoordinate* Coordinates = NewObject<UMaterialExpressionTextureCoordinate>(Material);
            Coordinates->CoordinateIndex = UVChannel;
            Material->Expressions.Add(Coordinates);
            Sample->Coordinates.Connect(0, Coordinates);
        }
        if (Material->BlendMode == BLEND_Opaque)
            Material->BaseColor.Connect(0, Sample); // RGB
        else
            Material->EmissiveColor.Connect(0, Sample); // RGB
        if (Material->BlendMode == BLEND_Translucent)
            Material->Opacity.Connect(4, Sample); // Alpha
    }
    else
    {
        UMaterialExpressionConstant3Vector* Color = NewObject<UMaterialExpressionConstant3Vector>(Material);
        Color->Constant = FLinearColor::White;
        Material->Expressions.Add(Color);
        if (Material->BlendMode == BLEND_Opaque)
            Material->BaseColor.Connect(0, Color);
        else
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
    if (!MeshPath.IsEmpty())
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *MeshPath);
        if (!Mesh)
        {
            UE_LOG(LogTemp, Error, TEXT("Mesh did not load: %s"), *MeshPath);
            return 5;
        }
        Mesh->SetMaterial(0, Material);
        Mesh->PostEditChange();
        UPackage* MeshPackage = Mesh->GetOutermost();
        MeshPackage->MarkPackageDirty();
        const FString MeshFilename = FPackageName::LongPackageNameToFilename(
            MeshPackage->GetName(), FPackageName::GetAssetPackageExtension());
        if (!UPackage::SavePackage(MeshPackage, Mesh, RF_Standalone, *MeshFilename, GError))
        {
            UE_LOG(LogTemp, Error, TEXT("Could not save mesh with reconstructed material: %s"), *MeshFilename);
            return 6;
        }
        UE_LOG(LogTemp, Display, TEXT("Applied reconstructed material to %s"), *MeshPath);
    }
    return 0;
}
