#pragma once
#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"

// Draft: all methods and HTTP completions must execute on the game thread.
// Keep one owner per match. Membership is not proof of a connection's identity.
class FFortAthenaRegistry : public TSharedFromThis<FFortAthenaRegistry>
{
public:
    struct FConfig {
        FString BaseUrl, Secret, Address, Playlist, Build;
        int32 Port = 0, BeaconPort = 0, Capacity = 0;
    };
    using FRegistered = TFunction<void(const FString&, const TArray<uint8>&)>;
    using FPlayerLoadoutResponse = TFunction<void(bool /*bTransportOk*/, int32 /*HttpCode*/, const FString& /*Body*/)>;
    static TSharedPtr<FFortAthenaRegistry> Create(const FConfig& Config, FRegistered OnRegistered);
    void Tick(double Now);
    void SetState(const FString& Phase, bool bJoinable, const TArray<FString>& ConnectedIds);
    // Async; callback false on rejection, timeout, or transport failure.
    bool CheckAdmission(const FString& AccountId, double Now, TFunction<void(bool)> Completion);
    // Async authenticated GET /local/gameserver/<sid>/player/<account>; never logs body or secrets.
    bool FetchPlayerLoadout(const FString& AccountId, double Now, FPlayerLoadoutResponse Completion);
    void Shutdown();
    const FString& SessionId() const { return Session; }
    bool IsHealthy(double Now) const { return !bStopped && !bDead && !Session.IsEmpty() && LastSuccess > 0 && Now - LastSuccess < 25.0; }
private:
    FConfig Config;
    FRegistered OnRegistered;
    FString Session, Phase = TEXT("pending");
    TArray<FString> Connected;
    bool bJoinable = false, bStopped = false, bDead = false;
    double NextAttempt = 0, Deadline = 0, LastSuccess = 0;
    uint64 Generation = 0;
    FHttpRequestPtr Request;
    struct FAdmission { FHttpRequestPtr Request; double Deadline; TFunction<void(bool)> Completion; };
    struct FPendingPlayerRequest { FHttpRequestPtr Request; double Deadline; FPlayerLoadoutResponse Completion; };
    TMap<uint64, TSharedPtr<FAdmission>> Admissions;
    TMap<uint64, TSharedPtr<FPendingPlayerRequest>> PlayerRequests;
    uint64 NextAdmission = 0;
    uint64 NextPlayerRequest = 0;
    void Start(double Now);
    void Complete(uint64 Serial, FHttpRequestPtr Req, FHttpResponsePtr Response, bool bOk);
};
