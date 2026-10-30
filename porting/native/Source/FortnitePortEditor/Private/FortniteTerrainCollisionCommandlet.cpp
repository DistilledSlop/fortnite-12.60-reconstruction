#include "FortniteTerrainCollisionCommandlet.h"

#include "Engine/StaticMesh.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/Package.h"

UFortniteTerrainCollisionCommandlet::UFortniteTerrainCollisionCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
}

int32 UFortniteTerrainCollisionCommandlet::Main(const FString& Params)
{
    FString Tile;
    if (!FParse::Value(*Params, TEXT("Tile="), Tile) || Tile.Len() != 2 ||
        Tile[0] < TEXT('A') || Tile[0] > TEXT('H') ||
        Tile[1] < TEXT('1') || Tile[1] > TEXT('8'))
    {
        UE_LOG(LogTemp, Error, TEXT("Use -Tile=A1 through -Tile=H8 for an imported landscape mesh"));
        return 2;
    }
    const FString Name = FString::Printf(TEXT("LandscapeStreamingProxy_%s"), *Tile);
    const FString PackagePath = FString::Printf(
        TEXT("/Game/Athena/Apollo/Maps/Landscape/Converted/%s"), *Name);
    const FString ObjectPath = PackagePath + TEXT(".") + Name;
    UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, *ObjectPath);
    if (!Mesh)
    {
        UE_LOG(LogTemp, Error, TEXT("Converted terrain mesh not found: %s"), *ObjectPath);
        return 3;
    }
    Mesh->CreateBodySetup();
    Mesh->BodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
    Mesh->Build(true);
#if WITH_PHYSX
    UE_LOG(LogTemp, Display, TEXT("Terrain collision built: %s triangle_meshes=%d"),
        *ObjectPath, Mesh->BodySetup->TriMeshes.Num());
#endif
    UPackage* Package = Mesh->GetOutermost();
    Package->MarkPackageDirty();
    const FString Filename = FPackageName::LongPackageNameToFilename(
        PackagePath, FPackageName::GetAssetPackageExtension());
    if (!UPackage::SavePackage(Package, Mesh, RF_Standalone, *Filename, GError))
    {
        UE_LOG(LogTemp, Error, TEXT("Could not save terrain collision mesh: %s"), *Filename);
        return 4;
    }
    UE_LOG(LogTemp, Display, TEXT("Saved terrain collision mesh: %s"), *Filename);
    return 0;
}
