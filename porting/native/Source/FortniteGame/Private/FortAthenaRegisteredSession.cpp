#include "FortAthenaRegisteredSession.h"
#include "FortAthenaRegistry.h"
#include "AFortGameModeAthena.h"
#include "AFortGameStateAthena.h"
#include "EAthenaGamePhase.h"
#include "Engine/NetConnection.h"
#include "Engine/NetDriver.h"
#include "IPAddress.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/Char.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Containers/Ticker.h"

namespace FortAthenaRegisteredSession
{
    struct FPeer
    {
        FString Account;
        uint64 Serial = 0;
        bool bApproved = false;
        bool bLogin = false;
    };
    struct FOwner : TSharedFromThis<FOwner>
    {
        TWeakObjectPtr<UWorld> World;
        TWeakObjectPtr<AFortGameModeAthena> GameMode;
        TSharedPtr<FFortAthenaRegistry> Registry;
        TArray<uint8> Key;
        TMap<TWeakObjectPtr<UNetConnection>, FPeer> Peers;
        uint64 Serial = 0;
        int32 ListenPort = 0;
        bool bOpen = false;
        bool bStopped = false;
        void Stop()
        {
            bStopped = true; bOpen = false;
            if (Registry) Registry->Shutdown();
            Registry.Reset(); Key.Empty(); Peers.Empty();
        }
        bool Healthy() const { return !bStopped && Registry && Key.Num() == 32 && Registry->IsHealthy(FPlatformTime::Seconds()); }
        bool Open() const
        {
            AFortGameModeAthena* Mode = GameMode.Get();
            AFortGameStateAthena* State = Mode ? Mode->GetGameState<AFortGameStateAthena>() : nullptr;
            UNetDriver* Driver = World.IsValid() ? World->GetNetDriver() : nullptr;
            TSharedPtr<const FInternetAddr> Address = Driver ? Driver->GetLocalAddr() : nullptr;
            return Healthy() && Driver && Driver->IsServer() && Address.IsValid() && Address->GetPort() == ListenPort && State && (State->GamePhase == EAthenaGamePhase::None || State->GamePhase == EAthenaGamePhase::Setup || State->GamePhase == EAthenaGamePhase::Warmup);
        }
        void Tick()
        {
            if (bStopped || !Registry) return;
            const double Now = FPlatformTime::Seconds();
            Registry->Tick(Now);
            if (!Registry->SessionId().IsEmpty() && !Registry->IsHealthy(Now)) { Stop(); return; }
            AFortGameModeAthena* Mode = GameMode.Get();
            AFortGameStateAthena* State = Mode ? Mode->GetGameState<AFortGameStateAthena>() : nullptr;
            const bool bWaiting = State && (State->GamePhase == EAthenaGamePhase::None || State->GamePhase == EAthenaGamePhase::Setup || State->GamePhase == EAthenaGamePhase::Warmup);
            bOpen = Open();
            TArray<FString> Connected;
            for (auto It = Peers.CreateIterator(); It; ++It)
            {
                UNetConnection* Connection = It.Key().Get();
                FPeer& Peer = It.Value();
                if (!Connection || Connection->State == USOCK_Closed) { It.RemoveCurrent(); continue; }
                APlayerController* PC = Connection->PlayerController;
                APlayerState* PS = PC ? PC->PlayerState : nullptr;
                if (!Peer.bLogin || !PS) continue;
                const FUniqueNetIdRepl& Id = PS->GetUniqueId();
                if (!Id.IsValid() || !Id->ToString().Equals(Peer.Account, ESearchCase::IgnoreCase))
                { Connection->Close(); It.RemoveCurrent(); continue; }
                Connected.Add(Peer.Account);
            }
            Connected.Sort();
            Registry->SetState(bWaiting ? TEXT("warmup") : TEXT("in_progress"), bOpen, Connected);
        }
    };
    static TSharedPtr<FOwner> Owner;
    static FDelegateHandle TickerHandle;
    static bool Validate(UNetConnection* Connection, const FString& Account, bool bSplit)
    {
        if (!Owner || !Owner->Open() || bSplit || !Connection || Connection->State == USOCK_Closed) return false;
        FPeer* Peer = Owner->Peers.Find(TWeakObjectPtr<UNetConnection>(Connection));
        if (!Peer || !Peer->bApproved || Peer->bLogin || !Account.Equals(Peer->Account, ESearchCase::IgnoreCase)) return false;
        Peer->bLogin = true;
        return true;
    }
    static bool Tick(float)
    {
        TSharedPtr<FOwner> Current = Owner;
        if (Current) Current->Tick();
        return true;
    }
    static void WorldCleanup(UWorld* World, bool, bool)
    {
        if (Owner && Owner->World.Get() == World) { Owner->Stop(); Owner.Reset(); }
    }
    bool Enabled() { return FParse::Param(FCommandLine::Get(), TEXT("FortniteRegisteredMatchmaking")); }
    void Initialize()
    {
        if (!Enabled()) return;
        FNetDelegates::OnValidateConnectionLogin.BindStatic(&Validate);
        TickerHandle = FTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&Tick));
        FWorldDelegates::OnWorldCleanup.AddStatic(&WorldCleanup);
    }
    static bool ReadInt(const TCHAR* Name, int32& Out)
    {
        FString S = FPlatformMisc::GetEnvironmentVariable(Name);
        if (S.IsEmpty() || !S.IsNumeric() || S.Len() > 5) return false;
        Out = FCString::Atoi(*S); return Out > 0;
    }
    void Start(AFortGameModeAthena* Mode)
    {
        if (!Enabled() || !Mode || !Mode->GetWorld() || Mode->GetWorld()->GetNetMode() != NM_DedicatedServer) return;
        if (Owner) Owner->Stop();
        Owner = MakeShared<FOwner>(); Owner->World = Mode->GetWorld(); Owner->GameMode = Mode;
        FFortAthenaRegistry::FConfig C;
        C.BaseUrl = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_URL"));
        C.Secret = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_SECRET"));
        C.Address = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_ADDRESS"));
        C.Playlist = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_PLAYLIST"));
        C.Build = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_BUILD"));
        if (!ReadInt(TEXT("FORTNITE_GAMESERVER_PORT"), C.Port) || !ReadInt(TEXT("FORTNITE_GAMESERVER_BEACON_PORT"), C.BeaconPort) || !ReadInt(TEXT("FORTNITE_GAMESERVER_CAPACITY"), C.Capacity)) return;
        Owner->ListenPort = C.Port;
        TWeakPtr<FOwner> Weak = Owner;
        Owner->Registry = FFortAthenaRegistry::Create(C, [Weak](const FString&, const TArray<uint8>& Key) {
            TSharedPtr<FOwner> Self = Weak.Pin(); if (Self && !Self->bStopped) Self->Key = Key;
        });
    }
    static bool HexId(const FString& S)
    {
        if (S.Len() != 32) return false;
        for (TCHAR C : S) if (!FChar::IsHexDigit(C)) return false;
        return true;
    }
    static void Fail(const FOnEncryptionKeyResponse& Reply)
    {
        Reply.ExecuteIfBound(FEncryptionKeyResponse(EEncryptionResponse::Failure, TEXT("Admission denied")));
    }
    void ReceiveToken(const FString& Token, const FOnEncryptionKeyResponse& Reply)
    {
        TSharedPtr<FOwner> Self = Owner;
        UNetConnection* Connection = Cast<UNetConnection>(Reply.GetUObject());
        if (!Self || !Self->Open() || !Connection || Connection->State == USOCK_Closed || Connection->GetWorld() != Self->World.Get() || Token.Len() != 65 || Token[32] != ':') { Fail(Reply); return; }
        // Preserve the backend's case-sensitive reservation identity.
        FString Account = Token.Left(32), Session = Token.Mid(33);
        if (!HexId(Account) || !HexId(Session) || !Session.Equals(Self->Registry->SessionId(), ESearchCase::IgnoreCase)) { Fail(Reply); return; }
        for (const auto& Pair : Self->Peers)
            if (Pair.Key.IsValid() && Pair.Key.Get()->State != USOCK_Closed && Pair.Value.Account == Account) { Fail(Reply); return; }
        FPeer Peer; Peer.Account = Account; Peer.Serial = ++Self->Serial;
        TWeakObjectPtr<UNetConnection> WeakConnection(Connection);
        Self->Peers.Add(WeakConnection, Peer);
        TWeakPtr<FOwner> WeakOwner = Self; const uint64 Serial = Peer.Serial;
        const bool bStarted = Self->Registry->CheckAdmission(Account, FPlatformTime::Seconds(), [WeakOwner, WeakConnection, Serial, Reply](bool bAllowed) {
            TSharedPtr<FOwner> Current = WeakOwner.Pin(); UNetConnection* Conn = WeakConnection.Get();
            if (!Current || !Conn) return;
            FPeer* Entry = Current->Peers.Find(WeakConnection);
            if (!Entry || Entry->Serial != Serial) return;
            if (!bAllowed || !Current->Open() || Conn->State == USOCK_Closed) { Current->Peers.Remove(WeakConnection); Fail(Reply); return; }
            Entry->bApproved = true;
            FEncryptionKeyResponse Response(EEncryptionResponse::Success, FString()); Response.EncryptionData.Key = Current->Key;
            Reply.ExecuteIfBound(Response);
        });
        if (!bStarted) { Self->Peers.Remove(WeakConnection); Fail(Reply); }
    }
}
