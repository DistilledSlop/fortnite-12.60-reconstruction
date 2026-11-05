#include "LocalAccountSubsystem.h"

#include "Http.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

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

    PendingRequest = FHttpModule::Get().CreateRequest();
    PendingRequest->SetURL(Origin + TEXT("/account/api/oauth/token"));
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
    PendingRequest.Reset();
    FString NewAccountId, NewDisplayName, NewAccessToken;
    TSharedPtr<FJsonObject> Json;
    const bool bValid = bConnected && Response.IsValid() && Response->GetResponseCode() >= 200 &&
        Response->GetResponseCode() < 300 &&
        FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Json) &&
        Json.IsValid() && Json->TryGetStringField(TEXT("account_id"), NewAccountId) && !NewAccountId.IsEmpty() &&
        Json->TryGetStringField(TEXT("displayName"), NewDisplayName) && !NewDisplayName.IsEmpty() &&
        Json->TryGetStringField(TEXT("access_token"), NewAccessToken) && !NewAccessToken.IsEmpty();
    if (bValid)
    {
        AccountId = MoveTemp(NewAccountId);
        DisplayName = MoveTemp(NewDisplayName);
        AccessToken = MoveTemp(NewAccessToken);
    }
    OnSelectionComplete.Broadcast(bValid, bValid ? FString() : TEXT("Local account selection failed."));
}

void ULocalAccountSubsystem::Deinitialize()
{
    if (PendingRequest.IsValid())
    {
        PendingRequest->OnProcessRequestComplete().Unbind();
        PendingRequest->CancelRequest();
        PendingRequest.Reset();
    }
    AccessToken.Empty();
    Super::Deinitialize();
}
