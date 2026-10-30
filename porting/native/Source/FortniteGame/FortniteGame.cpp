#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectGlobals.h"

class FFortniteGameModule : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();
        FCoreDelegates::OnPostEngineInit.AddStatic(&FFortniteGameModule::ProbeAsset);
        FWorldDelegates::OnPostWorldInitialization.AddStatic(&FFortniteGameModule::OnWorldInitialized);
    }

private:
    static void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues)
    {
        if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteProbeMapMeshes")))
            return;
        World->OnWorldBeginPlay.AddLambda([World]()
        {
            for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
            {
                UStaticMeshComponent* Component = It->GetStaticMeshComponent();
                if (!Component)
                    continue;
                const FBoxSphereBounds Bounds = Component->Bounds;
                if (Bounds.ContainsNaN())
                    UE_LOG(LogTemp, Error, TEXT("Invalid map mesh bounds: actor=%s mesh=%s origin=%s extent=%s radius=%g transform=%s local=%s"),
                        *It->GetPathName(), Component->GetStaticMesh() ? *Component->GetStaticMesh()->GetPathName() : TEXT("none"),
                        *Bounds.Origin.ToString(), *Bounds.BoxExtent.ToString(), Bounds.SphereRadius,
                        *Component->GetComponentTransform().ToString(),
                        Component->GetStaticMesh() ? *Component->GetStaticMesh()->GetBounds().ToString() : TEXT("none"));
            }
        });
    }

    static void ProbeAsset()
    {
        FString AssetPath;
        if (!FParse::Value(FCommandLine::Get(), TEXT("FortniteProbeAsset="), AssetPath))
        {
            return;
        }

        UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
        UE_LOG(LogTemp, Display, TEXT("Fortnite asset probe: %s -> %s"),
            *AssetPath, Asset ? *Asset->GetClass()->GetName() : TEXT("LOAD_FAILED"));
        if (const UTexture2D* Texture = Cast<UTexture2D>(Asset))
        {
            const int64 FirstMipBytes = Texture->PlatformData && Texture->PlatformData->Mips.Num() > 0
                ? Texture->PlatformData->Mips[0].BulkData.GetBulkDataSize() : 0;
            UE_LOG(LogTemp, Display, TEXT("Fortnite texture probe: %dx%d, %d mips, first mip %lld bytes"),
                Texture->GetSizeX(), Texture->GetSizeY(), Texture->GetNumMips(), FirstMipBytes);
        }
        if (const UStaticMesh* Mesh = Cast<UStaticMesh>(Asset))
        {
            const int32 LODs = Mesh->GetNumLODs();
            UE_LOG(LogTemp, Display, TEXT("Fortnite static mesh probe: %d LODs, LOD0 vertices %d"),
                LODs, LODs > 0 ? Mesh->GetNumVertices(0) : 0);
        }
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FFortniteGameModule, FortniteGame, "FortniteGame");
