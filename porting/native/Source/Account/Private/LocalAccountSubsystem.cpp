#include "LocalAccountSubsystem.h"

#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
bool ReadResponse(FHttpResponsePtr Response, bool bConnected, TSharedPtr<FJsonObject>& Json)
{
    return bConnected && Response.IsValid() && Response->GetResponseCode() >= 200 &&
        Response->GetResponseCode() < 300 &&
        FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Json) && Json.IsValid();
}

bool ReadRevision(const TSharedPtr<FJsonObject>& Object, const TCHAR* Field, int32& Revision)
{
    double Number;
    if (!Object->TryGetNumberField(Field, Number) || !FMath::IsFinite(Number) || Number < 0 || Number > MAX_int32)
    {
        return false;
    }
    Revision = static_cast<int32>(Number);
    return static_cast<double>(Revision) == Number;
}

bool ReadAthenaProfile(const TSharedPtr<FJsonObject>& Envelope, const FString& AccountId, FString& Snapshot)
{
    FString ProfileId, ChangeType, SnapshotAccountId, SnapshotProfileId;
    int32 Revision, BaseRevision, CommandRevision, SnapshotRevision, SnapshotCommandRevision;
    const TArray<TSharedPtr<FJsonValue>>* Changes = nullptr;
    const TSharedPtr<FJsonObject>* Profile = nullptr;
    const TSharedPtr<FJsonObject>* Items = nullptr;
    const TSharedPtr<FJsonObject>* Stats = nullptr;
    const TSharedPtr<FJsonObject>* Attributes = nullptr;
    if (!Envelope->TryGetStringField(TEXT("profileId"), ProfileId) || ProfileId != TEXT("athena") ||
        !ReadRevision(Envelope, TEXT("profileRevision"), Revision) ||
        !ReadRevision(Envelope, TEXT("profileChangesBaseRevision"), BaseRevision) || BaseRevision != Revision ||
        !ReadRevision(Envelope, TEXT("profileCommandRevision"), CommandRevision) ||
        !Envelope->TryGetArrayField(TEXT("profileChanges"), Changes) || Changes->Num() != 1 ||
        !(*Changes)[0].IsValid() || (*Changes)[0]->Type != EJson::Object)
    {
        return false;
    }
    const TSharedPtr<FJsonObject> Change = (*Changes)[0]->AsObject();
    if (!Change.IsValid() || !Change->TryGetStringField(TEXT("changeType"), ChangeType) ||
        ChangeType != TEXT("fullProfileUpdate") || !Change->TryGetObjectField(TEXT("profile"), Profile) ||
        !Profile->IsValid() || !(*Profile)->TryGetStringField(TEXT("accountId"), SnapshotAccountId) ||
        SnapshotAccountId != AccountId || !(*Profile)->TryGetStringField(TEXT("profileId"), SnapshotProfileId) ||
        SnapshotProfileId != ProfileId || !ReadRevision(*Profile, TEXT("rvn"), SnapshotRevision) ||
        SnapshotRevision != Revision || !ReadRevision(*Profile, TEXT("commandRevision"), SnapshotCommandRevision) ||
        SnapshotCommandRevision != CommandRevision || !(*Profile)->TryGetObjectField(TEXT("items"), Items) ||
        !Items->IsValid() || !(*Profile)->TryGetObjectField(TEXT("stats"), Stats) || !Stats->IsValid() ||
        !(*Stats)->TryGetObjectField(TEXT("attributes"), Attributes) || !Attributes->IsValid())
    {
        return false;
    }
    return FJsonSerializer::Serialize(Profile->ToSharedRef(), TJsonWriterFactory<>::Create(&Snapshot));
}
}

bool ULocalAccountSubsystem::SelectUsername(const FString& BackendBaseUrl, const FString& Username)
{
    if (PendingRequest.IsValid() || BackendBaseUrl.IsEmpty() || Username.IsEmpty())
    {
        return false;
    }

    FString Origin = BackendBaseUrl;
    while (Origin.EndsWith(TEXT("/")))
    {
        Origin.LeftChopInline(1);
    }
    if (Origin.IsEmpty())
    {
        return false;
    }

    PendingOrigin = MoveTemp(Origin);
    PendingRequest = FHttpModule::Get().CreateRequest();
    PendingRequest->SetURL(PendingOrigin + TEXT("/account/api/oauth/token"));
    PendingRequest->SetVerb(TEXT("POST"));
    PendingRequest->SetHeader(TEXT("Content-Type"), TEXT("application/x-www-form-urlencoded"));
    PendingRequest->SetContentAsString(TEXT("grant_type=local_username&username=") + FPlatformHttp::UrlEncode(Username));
    PendingRequest->OnProcessRequestComplete().BindUObject(this, &ULocalAccountSubsystem::HandleSelectionComplete);
    const FHttpRequestPtr Request = PendingRequest;
    if (!Request->ProcessRequest())
    {
        Request->OnProcessRequestComplete().Unbind();
        if (PendingRequest == Request)
        {
            PendingRequest.Reset();
            ClearCandidate();
        }
        return false;
    }
    return true;
}

void ULocalAccountSubsystem::HandleSelectionComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnected)
{
    if (Request != PendingRequest)
    {
        return;
    }
    TSharedPtr<FJsonObject> Json;
    if (!ReadResponse(Response, bConnected, Json) ||
        !Json->TryGetStringField(TEXT("account_id"), PendingAccountId) || PendingAccountId.IsEmpty() ||
        !Json->TryGetStringField(TEXT("displayName"), PendingDisplayName) || PendingDisplayName.IsEmpty() ||
        !Json->TryGetStringField(TEXT("access_token"), PendingAccessToken) || PendingAccessToken.IsEmpty())
    {
        PendingRequest.Reset();
        ClearCandidate();
        OnSelectionComplete.Broadcast(false, TEXT("Local account selection failed."));
        return;
    }

    PendingRequest = FHttpModule::Get().CreateRequest();
    PendingRequest->SetURL(PendingOrigin + TEXT("/fortnite/api/game/v2/profile/") +
        FPlatformHttp::UrlEncode(PendingAccountId) + TEXT("/client/QueryProfile?profileId=athena&rvn=-1"));
    PendingRequest->SetVerb(TEXT("POST"));
    PendingRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    PendingRequest->SetHeader(TEXT("Authorization"), TEXT("bearer ") + PendingAccessToken);
    PendingRequest->SetContentAsString(TEXT("{}"));
    PendingRequest->OnProcessRequestComplete().BindUObject(this, &ULocalAccountSubsystem::HandleProfileComplete);
    const FHttpRequestPtr ProfileRequest = PendingRequest;
    if (!ProfileRequest->ProcessRequest())
    {
        ProfileRequest->OnProcessRequestComplete().Unbind();
        if (PendingRequest == ProfileRequest)
        {
            PendingRequest.Reset();
            ClearCandidate();
            OnSelectionComplete.Broadcast(false, TEXT("Athena profile request failed."));
        }
    }
}

void ULocalAccountSubsystem::HandleProfileComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnected)
{
    if (Request != PendingRequest)
    {
        return;
    }
    TSharedPtr<FJsonObject> Json;
    FString Snapshot;
    const bool bValid = ReadResponse(Response, bConnected, Json) && ReadAthenaProfile(Json, PendingAccountId, Snapshot);
    if (bValid)
    {
        AccountId = MoveTemp(PendingAccountId);
        DisplayName = MoveTemp(PendingDisplayName);
        AccessToken = MoveTemp(PendingAccessToken);
        AthenaProfileJson = MoveTemp(Snapshot);
    }
    PendingRequest.Reset();
    ClearCandidate();
    OnSelectionComplete.Broadcast(bValid, bValid ? FString() : TEXT("Athena profile request failed."));
}

void ULocalAccountSubsystem::ClearCandidate()
{
    PendingOrigin.Empty();
    PendingAccountId.Empty();
    PendingDisplayName.Empty();
    PendingAccessToken.Empty();
}

void ULocalAccountSubsystem::Deinitialize()
{
    if (PendingRequest.IsValid())
    {
        PendingRequest->OnProcessRequestComplete().Unbind();
        PendingRequest->CancelRequest();
        PendingRequest.Reset();
    }
    ClearCandidate();
    AccessToken.Empty();
    AthenaProfileJson.Empty();
    Super::Deinitialize();
}
