#pragma once
#include "CoreMinimal.h"
#include "Engine/NetworkDelegates.h"
class AFortGameModeAthena;
namespace FortAthenaRegisteredSession
{
    bool Enabled();
    void Initialize();
    void Start(AFortGameModeAthena* GameMode);
    void ReceiveToken(const FString& Token, const FOnEncryptionKeyResponse& Reply);
}
