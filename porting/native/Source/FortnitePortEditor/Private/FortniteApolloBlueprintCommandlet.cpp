#include "FortniteApolloBlueprintCommandlet.h"

#include "AssetRegistryModule.h"
#include "Engine/Blueprint.h"
#include "Engine/SCS_Node.h"
#include "Engine/SimpleConstructionScript.h"
#include "AFortPoiManager.h"
#include "AFortAthenaMapInfo.h"
#include "FortniteMapClasses.h"
#include "HAL/FileManager.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Components/BoxComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Misc/PackageName.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

namespace
{
void EnsureComponent(UBlueprint* Blueprint, UClass* ComponentClass, FName VariableName)
{
    USimpleConstructionScript* Script = Blueprint->SimpleConstructionScript;
    for (const USCS_Node* Existing : Script->GetAllNodes())
    {
        if (Existing->GetVariableName() == VariableName)
            return;
    }
    USCS_Node* Node = Script->CreateNode(ComponentClass, VariableName);
    if (USCS_Node* Root = Script->GetDefaultSceneRootNode())
        Root->AddChildNode(Node);
    else
        Script->AddNode(Node);
}

bool EnsureRecoveredComponents(UBlueprint* Blueprint, const FString& PackagePath)
{
    if (PackagePath.EndsWith(TEXT("/BP_DisplayShadowProxy_01")))
    {
        // The map's original cooked instance buffer is incompatible with the
        // stock Linux serializer. Keep the class loadable until that buffer is
        // decoded and rebuilt in a native Linux asset. Blueprint SCS node
        // removal during an in-place update did not save cleanly, so require
        // a fresh generated package if an older replacement has that node.
        USimpleConstructionScript* Script = Blueprint->SimpleConstructionScript;
        for (const USCS_Node* Node : Script->GetAllNodes())
        {
            if (Node->GetVariableName() == TEXT("InstancedStaticMesh"))
            {
                UE_LOG(LogTemp, Error, TEXT("Shadow replacement has an incompatible mesh node; recreate this generated package"));
                return false;
            }
        }
        return true;
    }
    if (!PackagePath.EndsWith(TEXT("/BP_SafeZoneStorm")))
        return true;

    EnsureComponent(Blueprint, UBoxComponent::StaticClass(), TEXT("Box"));
    EnsureComponent(Blueprint, UPostProcessComponent::StaticClass(), TEXT("PostProcess"));
    const TCHAR* NamedMeshes[] = {
        TEXT("SM_CloudBottomBlend"), TEXT("SM_CloudRing_Mobile"),
        TEXT("SM_CloudRingTopBlend"), TEXT("SM_Storm_ScreenEffect"),
        TEXT("CloudsOuterRing"), TEXT("StaticMesh"),
        TEXT("StormRings"), TEXT("StormQuadMain"),
    };
    for (const TCHAR* Name : NamedMeshes)
        EnsureComponent(Blueprint, UStaticMeshComponent::StaticClass(), FName(Name));
    for (int32 Number = 1; Number <= 10; ++Number)
        EnsureComponent(Blueprint, UStaticMeshComponent::StaticClass(),
            FName(*FString::Printf(TEXT("StaticMesh%d"), Number)));
    for (int32 Number = 1; Number <= 16; ++Number)
        EnsureComponent(Blueprint, UStaticMeshComponent::StaticClass(),
            FName(*FString::Printf(TEXT("StormQuadMain%d"), Number)));
    return true;
}
}

UFortniteApolloBlueprintCommandlet::UFortniteApolloBlueprintCommandlet()
{
    IsClient = false;
    IsEditor = true;
    IsServer = false;
    LogToConsole = true;
}

int32 UFortniteApolloBlueprintCommandlet::Main(const FString& Params)
{
    const bool bUpdate = FParse::Param(*Params, TEXT("Update"));
    struct FReplacement
    {
        const TCHAR* PackageName;
        UClass* ParentClass;
    };
    const FReplacement Replacements[] = {
        { TEXT("/Game/ContentCreationTools/ShadowProxyGenerator/Blueprints/BP_DisplayShadowProxy_01"), AFortSkydivingShadowProxy::StaticClass() },
        { TEXT("/Game/Athena/SafeZone/BP_SafeZoneStorm"), AFortSafeZoneStorm::StaticClass() },
        { TEXT("/Game/Athena/Environments/Landscape/Blueprints/BP_SetupNamedPoiColors"), AFortPoiManager::StaticClass() },
        { TEXT("/Game/Athena/Balance/MapInfos/DefaultMapInfo"), AFortAthenaMapInfo::StaticClass() },
    };

    // Refuse to overwrite any content already present at an original asset path.
    for (const FReplacement& Spec : Replacements)
    {
        const FString PackagePath(Spec.PackageName);
        if (!FPackageName::IsValidLongPackageName(PackagePath))
        {
            UE_LOG(LogTemp, Error, TEXT("Invalid replacement package: %s"), Spec.PackageName);
            return 2;
        }
        const FString Filename = FPackageName::LongPackageNameToFilename(
            PackagePath, FPackageName::GetAssetPackageExtension());
        if (IFileManager::Get().FileExists(*Filename) && !bUpdate)
        {
            UE_LOG(LogTemp, Error, TEXT("Refusing to overwrite existing content without -Update: %s"), *Filename);
            return 3;
        }
    }

    for (const FReplacement& Spec : Replacements)
    {
        const FString PackagePath(Spec.PackageName);
        const FName AssetName(*FPackageName::GetShortName(PackagePath));
        UPackage* Package = CreatePackage(nullptr, *PackagePath);
        UBlueprint* Blueprint = FindObject<UBlueprint>(Package, *AssetName.ToString());
        if (!Blueprint && bUpdate)
            Blueprint = LoadObject<UBlueprint>(nullptr,
                *(PackagePath + TEXT(".") + AssetName.ToString()));
        const bool bCreated = !Blueprint;
        if (!Blueprint)
            Blueprint = FKismetEditorUtilities::CreateBlueprint(
                Spec.ParentClass, Package, AssetName, BPTYPE_Normal,
                UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
        if (!Blueprint)
        {
            UE_LOG(LogTemp, Error, TEXT("Could not create replacement Blueprint: %s"), *PackagePath);
            return 4;
        }
        if (Blueprint->ParentClass != Spec.ParentClass)
        {
            UE_LOG(LogTemp, Error, TEXT("Parent mismatch for %s; refusing to update"), *PackagePath);
            return 5;
        }
        if (!EnsureRecoveredComponents(Blueprint, PackagePath))
            return 6;
        FKismetEditorUtilities::CompileBlueprint(Blueprint);
        if (!Blueprint->GeneratedClass || Blueprint->Status == BS_Error)
        {
            UE_LOG(LogTemp, Error, TEXT("Could not compile replacement Blueprint: %s"), *PackagePath);
            return 7;
        }

        if (bCreated)
            FAssetRegistryModule::AssetCreated(Blueprint);
        Package->MarkPackageDirty();
        const FString Filename = FPackageName::LongPackageNameToFilename(
            PackagePath, FPackageName::GetAssetPackageExtension());
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);
        if (!UPackage::SavePackage(Package, Blueprint, RF_Standalone, *Filename, GError))
        {
            UE_LOG(LogTemp, Error, TEXT("Could not save replacement Blueprint: %s"), *Filename);
            return 8;
        }
        UE_LOG(LogTemp, Display, TEXT("Saved replacement Blueprint %s with parent %s"),
            *PackagePath, *Spec.ParentClass->GetPathName());
    }
    return 0;
}
