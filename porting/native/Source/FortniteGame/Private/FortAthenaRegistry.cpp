#include "FortAthenaRegistry.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "Misc/Base64.h"
#include "Misc/Char.h"
#include "HAL/PlatformTime.h"

namespace
{
    bool SafeAccountPathSegment(const FString& AccountId)
    {
        if (AccountId.IsEmpty() || AccountId.Len() > 128)
            return false;
        for (TCHAR C : AccountId)
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

    template <typename TMapType, typename TInvokeTimeout>
    void ExpirePendingRequests(TMapType& Map, double Now, const bool& bStopped, TInvokeTimeout&& OnTimeout)
    {
        TArray<uint64> Expired;
        for (const auto& Pair : Map)
            if (Now >= Pair.Value->Deadline)
                Expired.Add(Pair.Key);
        for (uint64 Id : Expired)
        {
            auto Item = Map.FindRef(Id);
            if (!Item)
                continue;
            Map.Remove(Id);
            Item->Request->OnProcessRequestComplete().Unbind();
            Item->Request->CancelRequest();
            OnTimeout(Item);
            if (bStopped)
                return;
        }
    }
}

TSharedPtr<FFortAthenaRegistry> FFortAthenaRegistry::Create(const FConfig& C, FRegistered Callback)
{
    // Loopback only. Do not accept DNS aliases, redirects, userinfo or remote HTTP.
    // UE's HTTP implementation must also be configured not to follow redirects.
    const FString HttpPort = C.BaseUrl.Mid(17);
    bool bValidHttpPort = HttpPort.Len() > 0 && HttpPort.Len() <= 5;
    for (TCHAR Character : HttpPort)
        bValidHttpPort &= Character >= '0' && Character <= '9';
    bValidHttpPort &= FCString::Atoi(*HttpPort) > 0 && FCString::Atoi(*HttpPort) <= 65535;
    if (!(C.BaseUrl.StartsWith(TEXT("http://127.0.0.1:"))) || !bValidHttpPort ||
        C.Secret.IsEmpty() || C.Secret.Contains(TEXT("\r")) || C.Secret.Contains(TEXT("\n")) ||
        C.Address.IsEmpty() || C.Playlist.IsEmpty() || C.Build.IsEmpty() ||
        C.Port < 1 || C.Port > 65535 || C.BeaconPort < 1 || C.BeaconPort > 65535 || C.Capacity < 1 || C.Capacity > 1000 || !Callback)
        return nullptr;
    TSharedPtr<FFortAthenaRegistry> Owner(new FFortAthenaRegistry());
    Owner->Config = C; Owner->OnRegistered = MoveTemp(Callback); return Owner;
}
void FFortAthenaRegistry::SetState(const FString& InPhase, bool InJoinable, const TArray<FString>& Ids)
{
    check(IsInGameThread());
    TSet<FString> Unique;
    for (const FString& Id : Ids) { if (Id.IsEmpty() || Unique.Contains(Id)) return; Unique.Add(Id); }
    if (Ids.Num() > Config.Capacity || InPhase.IsEmpty()) return;
    if (Phase == InPhase && bJoinable == InJoinable && Connected == Ids) return;
    Phase = InPhase; bJoinable = InJoinable; Connected = Ids;
    // Publish a newly closed lobby promptly after any outstanding request completes.
    NextAttempt = 0;
}
void FFortAthenaRegistry::Tick(double Now)
{
    check(IsInGameThread());
    if (bStopped || bDead) return;
    ExpirePendingRequests(Admissions, Now, bStopped, [](const TSharedPtr<FAdmission>& Item) {
        auto Done = MoveTemp(Item->Completion);
        if (Done) Done(false);
    });
    if (bStopped) return;
    ExpirePendingRequests(PlayerRequests, Now, bStopped, [](const TSharedPtr<FPendingPlayerRequest>& Item) {
        auto Done = MoveTemp(Item->Completion);
        if (Done) Done(false, 0, FString());
    });
    if (bStopped) return;
    if (Request.IsValid() && Now >= Deadline) {
        ++Generation; FHttpRequestPtr Old = MoveTemp(Request); Old->OnProcessRequestComplete().Unbind(); Old->CancelRequest(); NextAttempt = Now + 1;
    }
    if (!Request.IsValid() && Now >= NextAttempt) Start(Now);
}
void FFortAthenaRegistry::Start(double Now)
{
    const bool bRegister = Session.IsEmpty();
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>();
    if (bRegister) {
        Body->SetStringField(TEXT("address"), Config.Address); Body->SetNumberField(TEXT("port"), Config.Port);
        Body->SetNumberField(TEXT("beaconPort"), Config.BeaconPort); Body->SetNumberField(TEXT("maxPlayers"), Config.Capacity);
        Body->SetStringField(TEXT("buildUniqueId"), Config.Build);
        TArray<TSharedPtr<FJsonValue>> List; List.Add(MakeShared<FJsonValueString>(Config.Playlist)); Body->SetArrayField(TEXT("playlists"), List);
    } else {
        Body->SetStringField(TEXT("phase"), Phase); Body->SetBoolField(TEXT("joinable"), bJoinable);
        Body->SetNumberField(TEXT("numPlayers"), Connected.Num());
        TArray<TSharedPtr<FJsonValue>> List; for (const FString& Id : Connected) List.Add(MakeShared<FJsonValueString>(Id));
        Body->SetArrayField(TEXT("connectedPlayerIds"), List);
    }
    FString Payload; TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload); FJsonSerializer::Serialize(Body, Writer);
    Request = FHttpModule::Get().CreateRequest();
    Request->SetURL(Config.BaseUrl + (bRegister ? TEXT("/local/gameserver/register") : TEXT("/local/gameserver/") + Session + TEXT("/state")));
    Request->SetVerb(TEXT("POST")); Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Config.Secret);
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json")); Request->SetContentAsString(Payload);
    const uint64 Serial = ++Generation; Deadline = Now + 5; NextAttempt = Now + 10;
    TWeakPtr<FFortAthenaRegistry> Weak = AsShared();
    Request->OnProcessRequestComplete().BindLambda([Weak, Serial](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool Ok) {
        if (TSharedPtr<FFortAthenaRegistry> Self = Weak.Pin()) Self->Complete(Serial, Req, Resp, Ok);
    });
    FHttpRequestPtr Current = Request;
    if (!Current->ProcessRequest() && Request == Current) { Request.Reset(); NextAttempt = Now + 1; }
}
void FFortAthenaRegistry::Complete(uint64 Serial, FHttpRequestPtr Req, FHttpResponsePtr Response, bool bOk)
{
    check(IsInGameThread());
    if (bStopped || Serial != Generation || Req != Request) return;
    Request.Reset();
    if (!bOk || !Response.IsValid()) return;
    if (Response->GetResponseCode() == 404 && !Session.IsEmpty()) { LastSuccess = 0; bDead = true; return; }
    if (Response->GetResponseCode() < 200 || Response->GetResponseCode() >= 300) return;
    if (!Session.IsEmpty()) { LastSuccess = FPlatformTime::Seconds(); return; }
    TSharedPtr<FJsonObject> Json; TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    FString Id, Enc; TArray<uint8> Key;
    if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid() ||
        !Json->TryGetStringField(TEXT("sessionId"), Id) || Id.Len() != 32 ||
        !Json->TryGetStringField(TEXT("encryptionKey"), Enc) || !FBase64::Decode(Enc, Key) || Key.Num() != 32) return;
    for (TCHAR Ch : Id) if (!FChar::IsHexDigit(Ch)) return;
    Session = Id; LastSuccess = FPlatformTime::Seconds(); NextAttempt = 0;
    // Callback may synchronously shut down or drop the owner; no member access afterward.
    FRegistered Callback = OnRegistered; if (Callback) Callback(Id, Key);
}
bool FFortAthenaRegistry::CheckAdmission(const FString& AccountId, double Now, TFunction<void(bool)> Completion)
{
    check(IsInGameThread());
    if (!IsHealthy(Now) || AccountId.IsEmpty() || AccountId.Len() > 255 || !Completion) return false;
    TSharedRef<FJsonObject> Body = MakeShared<FJsonObject>(); Body->SetStringField(TEXT("accountId"), AccountId);
    FString Payload; TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Payload); FJsonSerializer::Serialize(Body, Writer);
    const uint64 Id = ++NextAdmission; TSharedPtr<FAdmission> Item = MakeShared<FAdmission>();
    Item->Deadline = Now + 5; Item->Completion = MoveTemp(Completion); Item->Request = FHttpModule::Get().CreateRequest();
    Item->Request->SetURL(Config.BaseUrl + TEXT("/local/gameserver/") + Session + TEXT("/admission"));
    Item->Request->SetVerb(TEXT("POST")); Item->Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Config.Secret);
    Item->Request->SetHeader(TEXT("Content-Type"), TEXT("application/json")); Item->Request->SetContentAsString(Payload);
    TWeakPtr<FFortAthenaRegistry> Weak = AsShared();
    Item->Request->OnProcessRequestComplete().BindLambda([Weak, Id](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool Ok) {
        TSharedPtr<FFortAthenaRegistry> Self = Weak.Pin(); if (!Self || Self->bStopped) return;
        TSharedPtr<FAdmission> Pending; if (!Self->Admissions.RemoveAndCopyValue(Id, Pending) || Pending->Request != Req) return;
        auto Done = MoveTemp(Pending->Completion); if (Done) Done(Ok && Resp.IsValid() && Resp->GetResponseCode() == 204);
    });
    Admissions.Add(Id, Item);
    if (!Item->Request->ProcessRequest()) {
        TSharedPtr<FAdmission> Pending;
        if (Admissions.RemoveAndCopyValue(Id, Pending)) { auto Done = MoveTemp(Pending->Completion); if (Done) Done(false); }
    }
    return true;
}
bool FFortAthenaRegistry::FetchPlayerLoadout(const FString& AccountId, double Now, FPlayerLoadoutResponse Completion)
{
    check(IsInGameThread());
    if (!IsHealthy(Now) || !SafeAccountPathSegment(AccountId) || !Completion) return false;
    const uint64 Id = ++NextPlayerRequest;
    TSharedPtr<FPendingPlayerRequest> Item = MakeShared<FPendingPlayerRequest>();
    Item->Deadline = Now + 5;
    Item->Completion = MoveTemp(Completion);
    Item->Request = FHttpModule::Get().CreateRequest();
    Item->Request->SetURL(Config.BaseUrl + TEXT("/local/gameserver/") + Session + TEXT("/player/") + AccountId);
    Item->Request->SetVerb(TEXT("GET"));
    Item->Request->SetHeader(TEXT("Authorization"), TEXT("Bearer ") + Config.Secret);
    TWeakPtr<FFortAthenaRegistry> Weak = AsShared();
    Item->Request->OnProcessRequestComplete().BindLambda([Weak, Id](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool Ok) {
        TSharedPtr<FFortAthenaRegistry> Self = Weak.Pin();
        if (!Self || Self->bStopped) return;
        TSharedPtr<FPendingPlayerRequest> Pending;
        if (!Self->PlayerRequests.RemoveAndCopyValue(Id, Pending) || Pending->Request != Req) return;
        auto Done = MoveTemp(Pending->Completion);
        if (!Done) return;
        const bool bValidResp = Ok && Resp.IsValid();
        Done(bValidResp, bValidResp ? Resp->GetResponseCode() : 0, bValidResp ? Resp->GetContentAsString() : FString());
    });
    PlayerRequests.Add(Id, Item);
    if (!Item->Request->ProcessRequest()) {
        TSharedPtr<FPendingPlayerRequest> Pending;
        if (PlayerRequests.RemoveAndCopyValue(Id, Pending)) {
            auto Done = MoveTemp(Pending->Completion);
            if (Done) Done(false, 0, FString());
        }
    }
    return true;
}
void FFortAthenaRegistry::Shutdown()
{
    check(IsInGameThread()); if (bStopped) return; bStopped = true; ++Generation;
    FHttpRequestPtr Old = MoveTemp(Request); if (Old.IsValid()) { Old->OnProcessRequestComplete().Unbind(); Old->CancelRequest(); }
    for (auto& Pair : Admissions) { Pair.Value->Request->OnProcessRequestComplete().Unbind(); Pair.Value->Request->CancelRequest(); }
    Admissions.Empty();
    for (auto& Pair : PlayerRequests) { Pair.Value->Request->OnProcessRequestComplete().Unbind(); Pair.Value->Request->CancelRequest(); }
    PlayerRequests.Empty();
    OnRegistered = nullptr; Config.Secret.Empty(); Session.Empty(); Connected.Empty();
}
