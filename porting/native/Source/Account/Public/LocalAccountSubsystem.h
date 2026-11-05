#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/IHttpRequest.h"
#include "LocalAccountSubsystem.generated.h"

class FUserOnlineAccount;
class FUniqueNetId;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLocalAccountSelectionComplete, bool, bSuccess, const FString&, Error);

/** Selects a local backend account. This does not sign in to the online subsystem. */
UCLASS()
class ACCOUNT_API ULocalAccountSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, Category = "Local Account")
    FLocalAccountSelectionComplete OnSelectionComplete;

    UPROPERTY(BlueprintReadOnly, Category = "Local Account")
    FString AccountId;

    UPROPERTY(BlueprintReadOnly, Category = "Local Account")
    FString DisplayName;

    UPROPERTY(BlueprintReadOnly, Category = "Local Account")
    FString AccessToken;

    /** Validated full Athena profile JSON for the selected account. This is a snapshot,
     * not an instance of the original MCP profile manager. */
    UPROPERTY(BlueprintReadOnly, Category = "Local Account")
    FString AthenaProfileJson;

    /** BackendBaseUrl is the service origin, for example http://127.0.0.1:5595.
     * Returns false when a request is already pending or cannot be started. */
    UFUNCTION(BlueprintCallable, Category = "Local Account")
    bool SelectUsername(const FString& BackendBaseUrl, const FString& Username);

    UFUNCTION(BlueprintPure, Category = "Local Account")
    bool IsSelectionPending() const { return PendingRequest.IsValid(); }

    /** Materialize the selected account for an identity provider. The caller's
     * real provider ID must be valid and match AccountId; no ID type is inferred.
     * Returns an owned snapshot, independent of later selections. This does not
     * install an identity provider, change login status or broadcast completion.
     * Call on the game thread, like account selection. */
    TSharedPtr<FUserOnlineAccount> CreateSelectedOnlineUserAccount(
        const TSharedRef<const FUniqueNetId>& UserId, FString& Error) const;

    virtual void Deinitialize() override;

private:
    void HandleSelectionComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnected);
    void HandleProfileComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnected);
    void ClearCandidate();
    FHttpRequestPtr PendingRequest;
    FString PendingOrigin;
    FString PendingAccountId;
    FString PendingDisplayName;
    FString PendingAccessToken;
};
