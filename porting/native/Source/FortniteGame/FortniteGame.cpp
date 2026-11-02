#include "Engine/NetworkDelegates.h"
#include "AFortPlayerPawn.h"
#include "UFortRuntimeOptions.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/Base64.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/FileHelper.h"
#include "Misc/NetworkVersion.h"
#include "Misc/Parse.h"
#include "Modules/ModuleManager.h"
#include "PhysicsEngine/BodySetup.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UObjectHash.h"
#include "UObject/UnrealType.h"
#include "Misc/Paths.h"
#include "Misc/PackageName.h"
#include "LegacyMaterialManifestSHA256.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortniteNet, Log, All);

// Private/NetSerializeSelfTest.cpp: round-trip checks of the recovered struct_net bodies. Runs only with -NetSerializeSelfTest
// and exits the process (0 when every case passes, 1 otherwise).
void RunNetSerializeSelfTest();
void RegisterContextRuntimeProbeDriver();
void RegisterActivationBaseProbeDriver();
void RegisterButtonInternalProbeDriver();
void RegisterMainButtonStateProbeDriver();
void RegisterPanelIntroProbeDriver();
void RegisterPanelOutroProbeDriver();
void RegisterConfirmationDescriptionProbeDriver();
void RegisterRootWidgetInstanceProbeDriver();
void RegisterCommonInputActionRowProbeDriver();
void RegisterCommonActionWidgetFlowProbeDriver();
void RegisterCommonButtonProductionFlowProbeDriver();
void RegisterBorderDelegateFlowProbeDriver();
void RegisterCommonInputCurrentTypeProbeDriver();
void RegisterActionBindingDataProbeDriver();
void RegisterActionBindingProviderProbeDriver();
void RegisterActionBindingCreationProbeDriver();
void RegisterCommonButtonTextStyleProbeDriver();
void RegisterCommonTextBlockStyleProbeDriver();
void RegisterCommonTextTransformProbeDriver();
void RegisterFortGlobalsErebusProbeDriver();
void RegisterButtonSizeProbeDriver();
void RegisterStyleRebuildProbeDriver();
void RegisterBaseClickProbeDriver();
void RunOriginalVulkanShaderProbe();
void RunLegacyMaterialShaderMapProbe();
void RegisterLegacyOriginalMaterialProvider();
void ProbeLegacyOriginalMaterial();

// Network version checksum the original 12.60 client sends in NMT_Hello
// (client log: "Checksum: 683914758").
static constexpr uint32 FortniteNetworkChecksum = 683914758;
static constexpr int32 FortniteEncryptionKeyBytes = 32;

static int32 HexDigitValue(TCHAR Digit)
{
    if (Digit >= TEXT('0') && Digit <= TEXT('9')) return Digit - TEXT('0');
    if (Digit >= TEXT('a') && Digit <= TEXT('f')) return Digit - TEXT('a') + 10;
    if (Digit >= TEXT('A') && Digit <= TEXT('F')) return Digit - TEXT('A') + 10;
    return -1;
}

// Loads the 32-byte AES key from FORTNITE_NET_ENCRYPTION_KEY (or from the file
// named by FORTNITE_NET_ENCRYPTION_KEY_FILE). The value is 64 hex characters or
// base64. The key is never written to the log and never stored in the project.
static bool LoadNetworkEncryptionKey(TArray<uint8>& OutKey)
{
    FString Text = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_NET_ENCRYPTION_KEY"));
    if (Text.IsEmpty())
    {
        const FString KeyFile = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_NET_ENCRYPTION_KEY_FILE"));
        if (KeyFile.IsEmpty())
        {
            return false;
        }
        if (!FFileHelper::LoadFileToString(Text, *KeyFile))
        {
            UE_LOG(LogFortniteNet, Error, TEXT("Cannot read FORTNITE_NET_ENCRYPTION_KEY_FILE."));
            return false;
        }
    }
    Text = Text.TrimStartAndEnd();

    OutKey.Reset();
    if (Text.Len() == FortniteEncryptionKeyBytes * 2)
    {
        bool bHex = true;
        for (int32 Index = 0; Index < Text.Len() && bHex; ++Index)
        {
            bHex = HexDigitValue(Text[Index]) >= 0;
        }
        if (bHex)
        {
            for (int32 Index = 0; Index < FortniteEncryptionKeyBytes; ++Index)
            {
                OutKey.Add(static_cast<uint8>(HexDigitValue(Text[Index * 2]) << 4 | HexDigitValue(Text[Index * 2 + 1])));
            }
        }
    }
    if (OutKey.Num() == 0 && !FBase64::Decode(Text, OutKey))
    {
        OutKey.Reset();
    }
    if (OutKey.Num() != FortniteEncryptionKeyBytes)
    {
        UE_LOG(LogFortniteNet, Error, TEXT("Network encryption key must decode to %d bytes (got %d)."), FortniteEncryptionKeyBytes, OutKey.Num());
        OutKey.Reset();
        return false;
    }
    return true;
}

static FEncryptionKeyResponse MakeConfiguredKeyResponse()
{
    FEncryptionKeyResponse Response(EEncryptionResponse::Failure, TEXT("No network encryption key configured"));
    TArray<uint8> Key;
    if (LoadNetworkEncryptionKey(Key))
    {
        Response.Response = EEncryptionResponse::Success;
        Response.ErrorMsg.Reset();
        Response.EncryptionData.Key = MoveTemp(Key);
    }
    return Response;
}

// Server side: every encryption token is answered with the configured key. Mapping
// tokens to per-session keys belongs to the reconstructed backend, not this module.
static void ReceivedNetworkEncryptionToken(const FString& EncryptionToken, const FOnEncryptionKeyResponse& Delegate)
{
    const FEncryptionKeyResponse Response = MakeConfiguredKeyResponse();
    if (Response.Response == EEncryptionResponse::Success)
    {
        UE_LOG(LogFortniteNet, Log, TEXT("Encryption token received (%d chars); returning a %d-byte key."), EncryptionToken.Len(), FortniteEncryptionKeyBytes);
    }
    else
    {
        UE_LOG(LogFortniteNet, Warning, TEXT("Encryption token received (%d chars) but no usable key is configured."), EncryptionToken.Len());
    }
    Delegate.Execute(Response);
}

// Client side: the ack path answers with the same configured key. This lets a local
// client and server share one key for testing. A shipping client takes its per-session
// key from the backend instead.
static void ReceivedNetworkEncryptionAck(const FOnEncryptionKeyResponse& Delegate)
{
    UE_LOG(LogFortniteNet, Log, TEXT("Encryption ack received; answering with the configured key."));
    Delegate.Execute(MakeConfiguredKeyResponse());
}

static uint32 GetFortniteNetworkVersion()
{
    UE_LOG(LogFortniteNet, Log, TEXT("Local network version checksum: %u (0x%08X)"), FortniteNetworkChecksum, FortniteNetworkChecksum);
    return FortniteNetworkChecksum;
}

// Accepts any remote version so the first real client's NMT_Hello value can be
// observed. Each distinct remote value is logged once.
static bool IsFortniteNetworkCompatible(uint32 LocalNetworkVersion, uint32 RemoteNetworkVersion)
{
    static TSet<uint32> SeenRemoteVersions;
    if (!SeenRemoteVersions.Contains(RemoteNetworkVersion))
    {
        SeenRemoteVersions.Add(RemoteNetworkVersion);
        UE_LOG(LogFortniteNet, Log, TEXT("Remote network version on first connection: %u (0x%08X), local %u, match=%d"),
            RemoteNetworkVersion, RemoteNetworkVersion, LocalNetworkVersion, RemoteNetworkVersion == LocalNetworkVersion ? 1 : 0);
    }
    return true;
}

static void BindNetworkEncryptionDelegate()
{
    // UGameInstance::Init binds its own handler during GEngine->Init, which runs after
    // StartupModule. Rebind after engine init so this server's key handler wins.
    FNetDelegates::OnReceivedNetworkEncryptionToken.BindStatic(&ReceivedNetworkEncryptionToken);
    FNetDelegates::OnReceivedNetworkEncryptionAck.BindStatic(&ReceivedNetworkEncryptionAck);
}

class FFortniteGameModule : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        FDefaultGameModuleImpl::StartupModule();
        RegisterContextRuntimeProbeDriver();
        RegisterActivationBaseProbeDriver();
        RegisterButtonInternalProbeDriver();
        RegisterMainButtonStateProbeDriver();
        RegisterCommonInputCurrentTypeProbeDriver();
        RegisterActionBindingDataProbeDriver();
        RegisterActionBindingProviderProbeDriver();
        RegisterActionBindingCreationProbeDriver();
        RegisterCommonButtonTextStyleProbeDriver();
        RegisterCommonTextBlockStyleProbeDriver();
        RegisterCommonTextTransformProbeDriver();
        RegisterFortGlobalsErebusProbeDriver();
        RegisterButtonSizeProbeDriver();
        RegisterStyleRebuildProbeDriver();
        RegisterBaseClickProbeDriver();
        RegisterPanelIntroProbeDriver();
        RegisterPanelOutroProbeDriver();
        RegisterConfirmationDescriptionProbeDriver();
        RegisterRootWidgetInstanceProbeDriver();
        RegisterCommonInputActionRowProbeDriver();
        RegisterCommonActionWidgetFlowProbeDriver();
        RegisterCommonButtonProductionFlowProbeDriver();
        RegisterBorderDelegateFlowProbeDriver();
        RegisterLegacyOriginalMaterialProvider();
        FNetworkVersion::GetLocalNetworkVersionOverride.BindStatic(&GetFortniteNetworkVersion);
        FNetworkVersion::IsNetworkCompatibleOverride.BindStatic(&IsFortniteNetworkCompatible);
        BindNetworkEncryptionDelegate();
        FCoreDelegates::OnPostEngineInit.AddStatic(&FFortniteGameModule::OnEngineInitForNetwork);
        FCoreDelegates::OnPostEngineInit.AddStatic(&FFortniteGameModule::ProbeAsset);
        FCoreDelegates::OnPostEngineInit.AddStatic(&RunOriginalVulkanShaderProbe);
        FCoreDelegates::OnPostEngineInit.AddStatic(&RunLegacyMaterialShaderMapProbe);
        FCoreDelegates::OnPostEngineInit.AddStatic(&ProbeLegacyOriginalMaterial);
        FCoreDelegates::OnPostEngineInit.AddStatic(&FFortniteGameModule::ProbeGameVersion);
        FWorldDelegates::OnPostWorldInitialization.AddStatic(&FFortniteGameModule::OnWorldInitialized);
        if (FParse::Param(FCommandLine::Get(), TEXT("NetSerializeSelfTest")))
        {
            RunNetSerializeSelfTest();
        }
    }

private:
    static void ProbeGameVersion()
    {
        if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteProbeGameVersion")))
            return;
        const UFortRuntimeOptions* Options = GetDefault<UFortRuntimeOptions>();
        UE_LOG(LogTemp, Display, TEXT("Fortnite game version probe: %s"), *Options->GetGameVersion());
    }

    static void OnEngineInitForNetwork()
    {
        BindNetworkEncryptionDelegate();
        auto LogCVar = [](const TCHAR* Name)
        {
            if (IConsoleVariable* Var = IConsoleManager::Get().FindConsoleVariable(Name))
            {
                UE_LOG(LogFortniteNet, Log, TEXT("%s = %s"), Name, *Var->GetString());
            }
            else
            {
                UE_LOG(LogFortniteNet, Warning, TEXT("%s is not registered"), Name);
            }
        };
        LogCVar(TEXT("net.MagicHeader"));
        LogCVar(TEXT("net.MaxChannelSize"));
        LogCVar(TEXT("net.AllowEncryption"));
        TArray<uint8> Key;
        UE_LOG(LogFortniteNet, Log, TEXT("Network encryption key %s."), LoadNetworkEncryptionKey(Key) ? TEXT("configured") : TEXT("not configured"));
        LogNativeClassLoads();
    }

    // Loads every promoted class by its /Script path, which is part of the wire format, and logs its
    // superclass. UObject names drop the C++ prefix letter: AFortPawn is /Script/FortniteGame.FortPawn. Both peers print these lines, so a missing class or a broken chain shows in the logs.
    // The list is the closure in porting/native_class_set.txt.
    static void LogNativeClassLoads()
    {
        static const TCHAR* ClassPaths[] = {
            TEXT("/Script/FortniteGame.FortGameStateAthena"),
            TEXT("/Script/FortniteGame.FortPlayerStateAthena"),
            TEXT("/Script/FortniteGame.FortPlayerControllerAthena"),
            TEXT("/Script/FortniteGame.FortPlayerPawnAthena"),
            TEXT("/Script/FortniteGame.FortWorldSettings"),
            TEXT("/Script/FortniteGame.FortAthenaMapInfo"),
            TEXT("/Script/FortniteGame.FortPoiManager"),
            TEXT("/Script/FortniteGame.FortWorldManager"),
            TEXT("/Script/FortniteGame.FortPlayerStateZone"),
            TEXT("/Script/FortniteGame.FortPlayerState"),
            TEXT("/Script/FortniteGame.FortGameStateZone"),
            TEXT("/Script/FortniteGame.FortGameState_InGame"),
            TEXT("/Script/FortniteGame.FortGameState"),
            TEXT("/Script/FortniteGame.FortGameStateBase"),
            TEXT("/Script/FortniteGame.FortPlayerControllerZone"),
            TEXT("/Script/FortniteGame.FortPlayerControllerGameplay"),
            TEXT("/Script/FortniteGame.FortPlayerController"),
            TEXT("/Script/FortniteGame.FortPlayerPawn"),
            TEXT("/Script/FortniteGame.FortPawn"),
            TEXT("/Script/PlayspaceFramework.PlayspaceGameState"),
            TEXT("/Script/PlayspaceFramework.PlayspacePlayerController"),
        };
        for (const TCHAR* Path : ClassPaths)
        {
            const UClass* Class = StaticLoadClass(UObject::StaticClass(), nullptr, Path);
            UE_LOG(LogFortniteNet, Log, TEXT("Native class %s: %s, super %s"), Path,
                Class ? TEXT("loaded") : TEXT("MISSING"),
                Class && Class->GetSuperClass() ? *Class->GetSuperClass()->GetPathName() : TEXT("none"));
        }
    }

    // Pawn readiness for the join smoke test (porting/server/smoke_join.sh). Every spawned pawn is logged on both peers:
    // the server with authority=1, the replicated copy on a client with authority=0.
    static void OnActorSpawnedForPawnLog(AActor* Actor)
    {
        if (Actor && Actor->IsA<AFortPlayerPawn>())
            UE_LOG(LogTemp, Display, TEXT("FortPlayerPawn ready: class=%s authority=%d"),
                *Actor->GetClass()->GetName(), Actor->HasAuthority() ? 1 : 0);
    }

    static void OnWorldInitialized(UWorld* World, const UWorld::InitializationValues)
    {
        World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateStatic(&FFortniteGameModule::OnActorSpawnedForPawnLog));
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

    static bool ProbeOriginalFloatChannel(UObject* Asset, FString& Error)
    {
        if (!Asset || !FParse::Param(FCommandLine::Get(),TEXT("FortniteLegacySequencer")) ||
            Asset->GetOutermost()->GetName()!=TEXT("/Game/UI/Login/SplashScreen/SplashScreenWidget"))
        { Error=TEXT("wrong source package or missing legacy sequencer switch"); return false; }
        FString PackageFilename;
        if (!FPackageName::DoesPackageExist(Asset->GetOutermost()->GetName(),nullptr,&PackageFilename) ||
            FPaths::GetExtension(PackageFilename)!=TEXT("uasset"))
        { Error=TEXT("could not resolve actual source package filename"); return false; }
        const FString ExportFilename=FPaths::ChangeExtension(PackageFilename,TEXT("uexp"));
        auto HashSource=[](const FString& Path, int64 ExpectedBytes, const TCHAR* ExpectedHash)->bool
        {
            if (IFileManager::Get().FileSize(*Path)!=ExpectedBytes) return false;
            TArray<uint8> Data; if (!FFileHelper::LoadFileToArray(Data,*Path) || Data.Num()!=ExpectedBytes) return false;
            uint8 Digest[32]; LegacyManifestSHA256(Data.GetData(),uint32(Data.Num()),Digest);
            return BytesToHex(Digest,32).Equals(ExpectedHash,ESearchCase::IgnoreCase);
        };
        if (!HashSource(PackageFilename,15958,TEXT("93645cada4aa03443fe94dbf095bc8b77680f9a7172bfc0e3c0725a980a7bd89")) ||
            !HashSource(ExportFilename,14683,TEXT("629573bd8d3d9fa6ed6faf198c5e3a7773444ce99e2707b7d7aa6a80f2bff89a")))
        { Error=TEXT("actual loaded package source bytes differ from original evidence"); return false; }
        UObject* Section=nullptr; int32 Matches=0;
        ForEachObjectWithOuter(Asset->GetOutermost(),[&](UObject* Object)
        {
            if (Object->GetName()!=TEXT("MovieSceneFloatSection_0") ||
                Object->GetClass()->GetPathName()!=TEXT("/Script/MovieSceneTracks.MovieSceneFloatSection")) return;
            UObject* Track=Object->GetOuter(); UObject* Scene=Track ? Track->GetOuter() : nullptr;
            UObject* Animation=Scene ? Scene->GetOuter() : nullptr; UObject* BP=Animation ? Animation->GetOuter() : nullptr;
            if (Track && Scene && Animation && BP && Track->GetName()==TEXT("MovieSceneFloatTrack_0") &&
                Scene->GetName()==TEXT("InputPromptIdle") && Animation->GetName()==TEXT("InputPromptIdle_INST") &&
                BP->GetName()==TEXT("SplashScreenWidget_C") && BP->GetOuter()==Asset->GetOutermost())
            { Section=Object; ++Matches; }
        },true);
        if (Matches!=1 || !Section) { Error=TEXT("original float section outer chain missing or ambiguous"); return false; }
        auto StructProperty=[](const UStruct* Type,const TCHAR* Name,const TCHAR* Expected)->FStructProperty*
        {
            FStructProperty* P=FindFProperty<FStructProperty>(Type,Name);
            return P && P->Struct && P->Struct->GetName()==Expected ? P : nullptr;
        };
        auto FloatValue=[](const UStruct* Type,const void* Data,const TCHAR* Name,float& Out)->bool
        {
            const FFloatProperty* P=FindFProperty<FFloatProperty>(Type,Name);
            if (!P) return false; Out=P->GetPropertyValue(P->ContainerPtrToValuePtr<void>(Data)); return true;
        };
        auto ByteValue=[](const UStruct* Type,const void* Data,const TCHAR* Name,uint8& Out)->bool
        {
            const FByteProperty* P=FindFProperty<FByteProperty>(Type,Name);
            if (!P) return false; Out=P->GetPropertyValue(P->ContainerPtrToValuePtr<void>(Data)); return true;
        };
        FStructProperty* Curve=StructProperty(Section->GetClass(),TEXT("FloatCurve"),TEXT("MovieSceneFloatChannel"));
        if (!Curve) { Error=TEXT("missing float channel property"); return false; }
        void* Channel=Curve->ContainerPtrToValuePtr<void>(Section);
        FArrayProperty* Times=FindFProperty<FArrayProperty>(Curve->Struct,TEXT("Times"));
        FArrayProperty* Values=FindFProperty<FArrayProperty>(Curve->Struct,TEXT("Values"));
        auto* TimeInner=Times ? CastField<FStructProperty>(Times->Inner) : nullptr;
        auto* ValueInner=Values ? CastField<FStructProperty>(Values->Inner) : nullptr;
        if (!TimeInner || !ValueInner || !TimeInner->Struct || !ValueInner->Struct ||
            TimeInner->Struct->GetName()!=TEXT("FrameNumber") || ValueInner->Struct->GetName()!=TEXT("MovieSceneFloatValue"))
        { Error=TEXT("float channel array element types differ"); return false; }
        FScriptArrayHelper T(Times,Times->ContainerPtrToValuePtr<void>(Channel));
        FScriptArrayHelper V(Values,Values->ContainerPtrToValuePtr<void>(Channel));
        if (T.Num()!=3 || V.Num()!=3) { Error=TEXT("original float channel key count differs"); return false; }
        const FIntProperty* Frame=FindFProperty<FIntProperty>(TimeInner->Struct,TEXT("Value"));
        FStructProperty* Tangent=StructProperty(ValueInner->Struct,TEXT("Tangent"),TEXT("MovieSceneTangentData"));
        if (!Frame || !Tangent) { Error=TEXT("float channel key structure differs"); return false; }
        const int32 ExpectedTimes[]={0,30000,60000}; const uint32 ExpectedValueBits[]={0x3f800000u,0u,0x3f800000u};
        FString Keys;
        for (int32 I=0;I<3;++I)
        {
            const int32 Time=Frame->GetPropertyValue(Frame->ContainerPtrToValuePtr<void>(T.GetRawPtr(I)));
            void* Key=V.GetRawPtr(I); void* TangentData=Tangent->ContainerPtrToValuePtr<void>(Key);
            float Value,Arrive,Leave,ArriveWeight,LeaveWeight; uint8 Interp,Mode,WeightMode;
            if (!FloatValue(ValueInner->Struct,Key,TEXT("Value"),Value) || !ByteValue(ValueInner->Struct,Key,TEXT("InterpMode"),Interp) ||
                !ByteValue(ValueInner->Struct,Key,TEXT("TangentMode"),Mode) ||
                !FloatValue(Tangent->Struct,TangentData,TEXT("ArriveTangent"),Arrive) || !FloatValue(Tangent->Struct,TangentData,TEXT("LeaveTangent"),Leave) ||
                !FloatValue(Tangent->Struct,TangentData,TEXT("ArriveTangentWeight"),ArriveWeight) || !FloatValue(Tangent->Struct,TangentData,TEXT("LeaveTangentWeight"),LeaveWeight) ||
                !ByteValue(Tangent->Struct,TangentData,TEXT("TangentWeightMode"),WeightMode))
            { Error=TEXT("float channel key property type differs"); return false; }
            uint32 Bits[5]; FMemory::Memcpy(&Bits[0],&Value,4); FMemory::Memcpy(&Bits[1],&Arrive,4); FMemory::Memcpy(&Bits[2],&Leave,4);
            FMemory::Memcpy(&Bits[3],&ArriveWeight,4); FMemory::Memcpy(&Bits[4],&LeaveWeight,4);
            if (Time!=ExpectedTimes[I] || Bits[0]!=ExpectedValueBits[I] || Bits[1] || Bits[2] || Bits[3] || Bits[4] || Interp!=2 || Mode!=0 || WeightMode!=0)
            { Error=FString::Printf(TEXT("original float channel key %d differs"),I); return false; }
            Keys+=FString::Printf(TEXT("%s{\"time\":%d,\"value_bits\":\"%08x\",\"interp\":%u,\"tangent_mode\":%u,\"arrive_bits\":\"%08x\",\"leave_bits\":\"%08x\",\"weight_mode\":%u,\"arrive_weight_bits\":\"%08x\",\"leave_weight_bits\":\"%08x\"}"),I ? TEXT(",") : TEXT(""),Time,Bits[0],Interp,Mode,Bits[1],Bits[2],WeightMode,Bits[3],Bits[4]);
        }
        FString Artifact;
        if (!FParse::Value(FCommandLine::Get(),TEXT("FortniteLegacyFloatChannelArtifact="),Artifact) || Artifact.IsEmpty() || FPaths::FileExists(Artifact))
        { Error=TEXT("float channel probe requires a new artifact path"); return false; }
        const FString Json=FString::Printf(TEXT("{\"schema\":\"fortnite.original-float-channel-runtime.v1\",\"source_package\":\"/Game/UI/Login/SplashScreen/SplashScreenWidget\",\"section\":\"%s\",\"section_class\":\"%s\",\"package_sha256\":\"93645cada4aa03443fe94dbf095bc8b77680f9a7172bfc0e3c0725a980a7bd89\",\"export_sha256\":\"629573bd8d3d9fa6ed6faf198c5e3a7773444ce99e2707b7d7aa6a80f2bff89a\",\"keys\":[%s],\"evidence_receipt_sha256\":\"fb652f574d2aa1335a9db7dcd5c33af2f2fd3b270498acafa1268ec732593bec\"}\n"),*Section->GetPathName(),*Section->GetClass()->GetPathName(),*Keys);
        if (!FFileHelper::SaveStringToFile(Json,*Artifact)) { Error=TEXT("failed writing float channel artifact"); return false; }
        UE_LOG(LogTemp,Display,TEXT("Original float channel probe passed: %s"),*Section->GetPathName());
        return true;
    }

    static void ProbeAsset()
    {
        FString AssetPath;
        if (!FParse::Value(FCommandLine::Get(), TEXT("FortniteProbeAsset="), AssetPath))
        {
            if (FParse::Param(FCommandLine::Get(),TEXT("FortniteProbeLegacyFloatChannel")))
            {
                UE_LOG(LogTemp,Error,TEXT("Original float channel probe requires FortniteProbeAsset"));
                FPlatformMisc::RequestExitWithStatus(false,1);
            }
            return;
        }

        UObject* Asset = StaticLoadObject(UObject::StaticClass(), nullptr, *AssetPath);
        UE_LOG(LogTemp, Display, TEXT("Fortnite asset probe: %s -> %s"),
            *AssetPath, Asset ? *Asset->GetClass()->GetName() : TEXT("LOAD_FAILED"));
        if (FParse::Param(FCommandLine::Get(),TEXT("FortniteProbeLegacyFloatChannel")))
        {
            FString Error;
            const bool bPassed=ProbeOriginalFloatChannel(Asset,Error);
            if (!bPassed) UE_LOG(LogTemp,Error,TEXT("Original float channel probe failed: %s"),*Error);
            FPlatformMisc::RequestExitWithStatus(false,bPassed ? 0 : 1);
            return;
        }
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
            if (const UBodySetup* Body = Mesh->BodySetup)
                UE_LOG(LogTemp, Display, TEXT("Fortnite static mesh collision: trace=%d boxes=%d spheres=%d convex=%d"),
                    static_cast<int32>(Body->CollisionTraceFlag), Body->AggGeom.BoxElems.Num(),
                    Body->AggGeom.SphereElems.Num(), Body->AggGeom.ConvexElems.Num());
        }
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FFortniteGameModule, FortniteGame, "FortniteGame");
