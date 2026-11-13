#include "FortAthenaRegisteredSession.h"
#include "FortAthenaCharacterCache.h"
#include "FortAthenaRegistry.h"
#include "AFortGameModeAthena.h"
#include "AFortGameStateAthena.h"
#include "UFortPlaylistAthena.h"
#include "EAthenaGamePhase.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
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
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace FortAthenaRegisteredSession
{
    static bool SafeTemplateNameSegment(const FString& Value)
    {
        if (Value.IsEmpty() || Value.Len() > 160 || Value.Contains(TEXT("..")))
            return false;
        for (TCHAR C : Value)
        {
            const bool bOk = (C >= TEXT('0') && C <= TEXT('9')) ||
                             (C >= TEXT('a') && C <= TEXT('z')) ||
                             (C >= TEXT('A') && C <= TEXT('Z')) ||
                             C == TEXT('_') || C == TEXT('-');
            if (!bOk)
                return false;
        }
        return true;
    }

    static bool ParseLoadoutResponse(
        const FString& Body,
        const FString& ExpectedAccount,
        FString& OutCharacterTemplateId,
        TArray<FVariantSelection>& OutVariants,
        FString& OutError)
    {
        OutCharacterTemplateId.Empty();
        OutVariants.Reset();

        TSharedPtr<FJsonObject> Root;
        if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Body), Root) || !Root.IsValid())
        {
            OutError = TEXT("malformed player loadout JSON");
            return false;
        }

        FString AccountId;
        if (!Root->TryGetStringField(TEXT("accountId"), AccountId) ||
            !AccountId.Equals(ExpectedAccount, ESearchCase::IgnoreCase))
        {
            OutError = TEXT("player loadout accountId mismatch");
            return false;
        }

        const TSharedPtr<FJsonObject>* CosmeticsObj = nullptr;
        if (!Root->TryGetObjectField(TEXT("cosmetics"), CosmeticsObj) || !CosmeticsObj || !CosmeticsObj->IsValid())
        {
            OutError = TEXT("missing cosmetics object in player loadout");
            return false;
        }

        const TArray<TSharedPtr<FJsonValue>>* CharacterArray = nullptr;
        if (!(*CosmeticsObj)->TryGetArrayField(TEXT("Character"), CharacterArray) || !CharacterArray || CharacterArray->Num() < 1)
        {
            OutError = TEXT("missing active Character slot in player loadout");
            return false;
        }

        FString RawTemplateId;
        if (!(*CharacterArray)[0].IsValid() || !(*CharacterArray)[0]->TryGetString(RawTemplateId) || RawTemplateId.IsEmpty())
        {
            OutError = TEXT("empty Character template ID in player loadout");
            return false;
        }

        const FString Prefix(TEXT("AthenaCharacter:"));
        if (!RawTemplateId.StartsWith(Prefix, ESearchCase::IgnoreCase))
        {
            OutError = TEXT("Character template ID lacks AthenaCharacter: prefix");
            return false;
        }
        const FString ShortName = RawTemplateId.Mid(Prefix.Len());
        if (!SafeTemplateNameSegment(ShortName))
        {
            OutError = TEXT("invalid Character template ID characters");
            return false;
        }
        OutCharacterTemplateId = Prefix + ShortName;

        const TSharedPtr<FJsonObject>* VariantsObj = nullptr;
        if (Root->HasField(TEXT("variants")))
        {
            if (!Root->TryGetObjectField(TEXT("variants"), VariantsObj) || !VariantsObj || !VariantsObj->IsValid())
            {
                OutError = TEXT("malformed variants field in player loadout");
                return false;
            }
            if ((*VariantsObj)->HasField(TEXT("Character")))
            {
                const TArray<TSharedPtr<FJsonValue>>* SlotVariants = nullptr;
                if (!(*VariantsObj)->TryGetArrayField(TEXT("Character"), SlotVariants) || !SlotVariants)
                {
                    OutError = TEXT("malformed Character variants array in player loadout");
                    return false;
                }
                if (SlotVariants->Num() > 0)
                {
                    const TSharedPtr<FJsonObject>* SlotEntry = nullptr;
                    if (!(*SlotVariants)[0].IsValid() || !(*SlotVariants)[0]->TryGetObject(SlotEntry) || !SlotEntry || !SlotEntry->IsValid())
                    {
                        OutError = TEXT("malformed Character variant slot object");
                        return false;
                    }
                    if ((*SlotEntry)->HasField(TEXT("variants")))
                    {
                        const TArray<TSharedPtr<FJsonValue>>* Channels = nullptr;
                        if (!(*SlotEntry)->TryGetArrayField(TEXT("variants"), Channels) || !Channels)
                        {
                            OutError = TEXT("malformed Character variant channels array");
                            return false;
                        }
                        for (const TSharedPtr<FJsonValue>& ChannelVal : *Channels)
                        {
                            const TSharedPtr<FJsonObject>* ChannelObj = nullptr;
                            if (!ChannelVal.IsValid() || !ChannelVal->TryGetObject(ChannelObj) || !ChannelObj || !ChannelObj->IsValid())
                            {
                                OutError = TEXT("malformed variant channel entry");
                                return false;
                            }
                            FVariantSelection Sel;
                            if (!(*ChannelObj)->TryGetStringField(TEXT("channel"), Sel.Channel) || Sel.Channel.IsEmpty() ||
                                !(*ChannelObj)->TryGetStringField(TEXT("active"), Sel.Active) || Sel.Active.IsEmpty())
                            {
                                OutError = TEXT("variant channel entry missing channel or active");
                                return false;
                            }
                            if ((*ChannelObj)->HasField(TEXT("owned")))
                            {
                                const TArray<TSharedPtr<FJsonValue>>* OwnedArr = nullptr;
                                if (!(*ChannelObj)->TryGetArrayField(TEXT("owned"), OwnedArr) || !OwnedArr)
                                {
                                    OutError = TEXT("malformed variant owned array");
                                    return false;
                                }
                                for (const TSharedPtr<FJsonValue>& OwnedVal : *OwnedArr)
                                {
                                    FString TagStr;
                                    if (!OwnedVal.IsValid() || !OwnedVal->TryGetString(TagStr) || TagStr.IsEmpty())
                                    {
                                        OutError = TEXT("malformed owned variant tag");
                                        return false;
                                    }
                                    Sel.Owned.Add(TagStr);
                                }
                            }
                            OutVariants.Add(MoveTemp(Sel));
                        }
                    }
                }
            }
        }
        return true;
    }

    struct FPeer
    {
        FString Account;
        uint64 Serial = 0;
        bool bApproved = false;
        bool bLogin = false;
        ELoadoutStatus LoadoutStatus = ELoadoutStatus::Pending;
        FString CharacterTemplateId;
        TArray<FVariantSelection> Variants;
        FString LoadoutError;
        bool bLoadoutApplied = false;
    };
    struct FOwner : TSharedFromThis<FOwner>
    {
        TWeakObjectPtr<UWorld> World;
        TWeakObjectPtr<AFortGameModeAthena> GameMode;
        TSharedPtr<FFortAthenaRegistry> Registry;
        TArray<uint8> Key;
        FString Playlist;
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
            return Healthy() && Driver && Driver->IsServer() && Address.IsValid() && Address->GetPort() == ListenPort && State && State->CurrentPlaylistInfo.BasePlaylist && State->CurrentPlaylistInfo.BasePlaylist->GetName().Equals(Playlist, ESearchCase::IgnoreCase) && (State->GamePhase == EAthenaGamePhase::None || State->GamePhase == EAthenaGamePhase::Setup || State->GamePhase == EAthenaGamePhase::Warmup);
        }
        void Tick()
        {
            if (bStopped || !Registry) return;
            const double Now = FPlatformTime::Seconds();
            const TSharedPtr<FFortAthenaRegistry> CurrentRegistry = Registry;
            CurrentRegistry->Tick(Now);
            if (bStopped || Registry != CurrentRegistry) return;
            if (!Registry->SessionId().IsEmpty() && !Registry->IsHealthy(Now)) { Stop(); return; }
            AFortGameModeAthena* Mode = GameMode.Get();
            AFortGameStateAthena* State = Mode ? Mode->GetGameState<AFortGameStateAthena>() : nullptr;
            const bool bWaiting = State && (State->GamePhase == EAthenaGamePhase::None || State->GamePhase == EAthenaGamePhase::Setup || State->GamePhase == EAthenaGamePhase::Warmup);
            bOpen = Open();
            TArray<FString> Connected;
            TArray<TWeakObjectPtr<APlayerController>> PendingApplyControllers;
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
                if (Peer.LoadoutStatus == ELoadoutStatus::Failed)
                {
                    UE_LOG(LogTemp, Error, TEXT("Registered character loadout failed for account %s (%s); closing connection"),
                        *Peer.Account, *Peer.LoadoutError);
                    Connection->Close();
                    It.RemoveCurrent();
                    continue;
                }
                if (Peer.LoadoutStatus == ELoadoutStatus::Ready && !Peer.bLoadoutApplied && PC)
                {
                    PendingApplyControllers.Add(PC);
                }
                Connected.Add(Peer.Account);
            }
            for (const TWeakObjectPtr<APlayerController>& WeakPC : PendingApplyControllers)
            {
                if (APlayerController* PC = WeakPC.Get())
                {
                    FortAthenaCharacterCache::ApplyForController(PC, false);
                }
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
        if (Peer->LoadoutStatus == ELoadoutStatus::Failed)
        {
            UE_LOG(LogTemp, Error, TEXT("Registered login denied for account %s: loadout failed (%s)"), *Peer->Account, *Peer->LoadoutError);
            return false;
        }
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
        FortAthenaCharacterCache::Prepare();
        FFortAthenaRegistry::FConfig C;
        C.BaseUrl = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_URL"));
        C.Secret = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_SECRET"));
        C.Address = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_ADDRESS"));
        C.Playlist = FPlatformMisc::GetEnvironmentVariable(TEXT("FORTNITE_GAMESERVER_PLAYLIST"));
        Owner->Playlist = C.Playlist;
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
    static void StartLoadoutFetch(
        const TSharedPtr<FOwner>& Current,
        const TWeakObjectPtr<UNetConnection>& WeakConnection,
        uint64 Serial,
        const FString& Account)
    {
        if (!Current || !Current->Registry || Current->bStopped)
            return;
        TWeakPtr<FOwner> WeakOwner = Current;
        const TSharedPtr<FFortAthenaRegistry> Registry = Current->Registry;
        const bool bStarted = Registry->FetchPlayerLoadout(
            Account,
            FPlatformTime::Seconds(),
            [WeakOwner, WeakConnection, Serial, Account](bool bTransportOk, int32 HttpCode, const FString& Body)
            {
                TSharedPtr<FOwner> OwnerRef = WeakOwner.Pin();
                UNetConnection* Conn = WeakConnection.Get();
                if (!OwnerRef || OwnerRef->bStopped || !Conn)
                    return;
                FPeer* Entry = OwnerRef->Peers.Find(WeakConnection);
                if (!Entry || Entry->Serial != Serial || !Entry->Account.Equals(Account, ESearchCase::IgnoreCase))
                    return;
                if (Conn->State == USOCK_Closed)
                {
                    OwnerRef->Peers.Remove(WeakConnection);
                    return;
                }
                if (!bTransportOk || HttpCode != 200)
                {
                    Entry->LoadoutStatus = ELoadoutStatus::Failed;
                    Entry->LoadoutError = bTransportOk
                        ? FString::Printf(TEXT("backend player loadout HTTP %d"), HttpCode)
                        : TEXT("backend player loadout transport failure or timeout");
                    UE_LOG(LogTemp, Error, TEXT("Registered loadout fetch failed for account %s: %s"),
                        *Entry->Account, *Entry->LoadoutError);
                    if (Entry->bLogin)
                    {
                        Conn->Close();
                        OwnerRef->Peers.Remove(WeakConnection);
                    }
                    return;
                }

                FString TemplateId;
                TArray<FVariantSelection> Variants;
                FString ParseError;
                if (!ParseLoadoutResponse(Body, Entry->Account, TemplateId, Variants, ParseError))
                {
                    Entry->LoadoutStatus = ELoadoutStatus::Failed;
                    Entry->LoadoutError = ParseError;
                    UE_LOG(LogTemp, Error, TEXT("Registered loadout parse rejected for account %s: %s"),
                        *Entry->Account, *Entry->LoadoutError);
                    if (Entry->bLogin)
                    {
                        Conn->Close();
                        OwnerRef->Peers.Remove(WeakConnection);
                    }
                    return;
                }

                Entry->CharacterTemplateId = MoveTemp(TemplateId);
                Entry->Variants = MoveTemp(Variants);
                Entry->LoadoutError.Empty();
                Entry->bLoadoutApplied = false;
                Entry->LoadoutStatus = ELoadoutStatus::Ready;
                UE_LOG(LogTemp, Display, TEXT("Registered loadout cached for account %s: %s (variants=%d)"),
                    *Entry->Account, *Entry->CharacterTemplateId, Entry->Variants.Num());

                if (Entry->bLogin && Conn->PlayerController && Conn->PlayerController->PlayerState)
                {
                    if (!Conn->PlayerController->GetPawn())
                    {
                        if (AFortGameModeAthena* Mode = OwnerRef->GameMode.Get())
                            Mode->RestartPlayer(Conn->PlayerController);
                    }
                    else
                        FortAthenaCharacterCache::ApplyForController(Conn->PlayerController, true);
                }
            });
        if (!bStarted)
        {
            if (FPeer* Entry = Current->Peers.Find(WeakConnection))
            {
                if (Entry->Serial == Serial)
                {
                    Entry->LoadoutStatus = ELoadoutStatus::Failed;
                    Entry->LoadoutError = TEXT("unable to start player loadout request");
                }
            }
        }
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
            const FString ApprovedAccount = Entry->Account;
            FEncryptionKeyResponse Response(EEncryptionResponse::Success, FString()); Response.EncryptionData.Key = Current->Key;
            Reply.ExecuteIfBound(Response);
            StartLoadoutFetch(Current, WeakConnection, Serial, ApprovedAccount);
        });
        if (!bStarted) { Self->Peers.Remove(WeakConnection); Fail(Reply); }
    }

    static FPeer* FindVerifiedPeer(const AController* Controller, FString& OutError)
    {
        if (!Enabled())
            return nullptr;
        if (!Owner || Owner->bStopped)
        {
            OutError = TEXT("registered session owner unavailable");
            return nullptr;
        }
        const APlayerController* PC = Cast<APlayerController>(Controller);
        UNetConnection* Conn = PC ? PC->GetNetConnection() : nullptr;
        if (!Conn || Conn->State == USOCK_Closed)
        {
            OutError = TEXT("controller has no active net connection");
            return nullptr;
        }
        FPeer* Peer = Owner->Peers.Find(TWeakObjectPtr<UNetConnection>(Conn));
        if (!Peer || !Peer->bApproved || !Peer->bLogin)
        {
            OutError = TEXT("connection lacks verified registered login");
            return nullptr;
        }
        if (PC->PlayerState)
        {
            const FUniqueNetIdRepl& Id = PC->PlayerState->GetUniqueId();
            if (Id.IsValid() && !Id->ToString().Equals(Peer->Account, ESearchCase::IgnoreCase))
            {
                OutError = TEXT("player state UniqueId mismatches verified peer");
                return nullptr;
            }
        }
        return Peer;
    }

    ELoadoutStatus QueryControllerLoadout(const AController* Controller, FResolvedPeerLoadout& OutLoadout, FString& OutError)
    {
        if (!Enabled())
            return ELoadoutStatus::NotRegisteredMode;
        FPeer* Peer = FindVerifiedPeer(Controller, OutError);
        if (!Peer)
            return ELoadoutStatus::Failed;
        if (Peer->LoadoutStatus == ELoadoutStatus::Pending)
            return ELoadoutStatus::Pending;
        if (Peer->LoadoutStatus == ELoadoutStatus::Failed)
        {
            OutError = Peer->LoadoutError;
            return ELoadoutStatus::Failed;
        }
        OutLoadout.AccountId = Peer->Account;
        OutLoadout.CharacterTemplateId = Peer->CharacterTemplateId;
        OutLoadout.Variants = Peer->Variants;
        OutLoadout.bApplied = Peer->bLoadoutApplied;
        return ELoadoutStatus::Ready;
    }

    void MarkControllerLoadoutApplied(const AController* Controller)
    {
        FString IgnoredError;
        if (FPeer* Peer = FindVerifiedPeer(Controller, IgnoredError))
        {
            Peer->bLoadoutApplied = true;
        }
    }

    void MarkControllerLoadoutFailed(const AController* Controller, const FString& Error)
    {
        if (!Owner)
            return;
        const APlayerController* PC = Cast<APlayerController>(Controller);
        UNetConnection* Conn = PC ? PC->GetNetConnection() : nullptr;
        if (!Conn)
            return;
        if (FPeer* Peer = Owner->Peers.Find(TWeakObjectPtr<UNetConnection>(Conn)))
        {
            Peer->LoadoutStatus = ELoadoutStatus::Failed;
            Peer->LoadoutError = Error;
            Peer->bLoadoutApplied = false;
        }
    }
}
