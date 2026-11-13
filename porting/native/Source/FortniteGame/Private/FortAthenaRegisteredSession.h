#pragma once
#include "CoreMinimal.h"
#include "Engine/NetworkDelegates.h"
class AController;
class AFortGameModeAthena;
namespace FortAthenaRegisteredSession
{
    enum class ELoadoutStatus : uint8
    {
        NotRegisteredMode,
        Pending,
        Ready,
        Failed
    };

    struct FVariantSelection
    {
        FString Channel;
        FString Active;
        TArray<FString> Owned;
    };

    struct FResolvedPeerLoadout
    {
        FString AccountId;
        FString CharacterTemplateId;
        TArray<FVariantSelection> Variants;
        bool bApplied = false;
    };

    bool Enabled();
    void Initialize();
    void Start(AFortGameModeAthena* GameMode);
    void ReceiveToken(const FString& Token, const FOnEncryptionKeyResponse& Reply);

    ELoadoutStatus QueryControllerLoadout(const AController* Controller, FResolvedPeerLoadout& OutLoadout, FString& OutError);
    void MarkControllerLoadoutApplied(const AController* Controller);
    void MarkControllerLoadoutFailed(const AController* Controller, const FString& Error);
}
