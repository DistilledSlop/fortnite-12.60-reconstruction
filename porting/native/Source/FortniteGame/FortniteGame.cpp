#include "Engine/NetworkDelegates.h"
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

DEFINE_LOG_CATEGORY_STATIC(LogFortniteNet, Log, All);

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
        FNetworkVersion::GetLocalNetworkVersionOverride.BindStatic(&GetFortniteNetworkVersion);
        FNetworkVersion::IsNetworkCompatibleOverride.BindStatic(&IsFortniteNetworkCompatible);
        BindNetworkEncryptionDelegate();
        FCoreDelegates::OnPostEngineInit.AddStatic(&FFortniteGameModule::OnEngineInitForNetwork);
        FCoreDelegates::OnPostEngineInit.AddStatic(&FFortniteGameModule::ProbeAsset);
        FWorldDelegates::OnPostWorldInitialization.AddStatic(&FFortniteGameModule::OnWorldInitialized);
    }

private:
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
    }


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
            if (const UBodySetup* Body = Mesh->BodySetup)
                UE_LOG(LogTemp, Display, TEXT("Fortnite static mesh collision: trace=%d boxes=%d spheres=%d convex=%d"),
                    static_cast<int32>(Body->CollisionTraceFlag), Body->AggGeom.BoxElems.Num(),
                    Body->AggGeom.SphereElems.Num(), Body->AggGeom.ConvexElems.Num());
        }
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FFortniteGameModule, FortniteGame, "FortniteGame");
