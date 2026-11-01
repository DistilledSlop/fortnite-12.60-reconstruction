#include "BlueprintContextRuntime.h"
#include "BlueprintContextCore.h"
#include "UBlueprintContextBase.h"
#include "Blueprint/UserWidget.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "TimerManager.h"
#include "UObject/GCObject.h"
#include "UObject/UObjectHash.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
bool ProbeEnabled() { return FParse::Param(FCommandLine::Get(), TEXT("ContextRuntimeProbe")); }
uint64 ProbeId(const void* P) { return (uint64)P; }
class FContextManager;
FContextManager* ObservedManager = nullptr; // non-owning; no singleton acquisition
bool ProbeLifecycleArmed = false;
bool ProbeSawReady = false;
bool ProbeSawCleanup = false;
class FContextManager final : public FGCObject, public TSharedFromThis<FContextManager>
{
public:
    using Object = UObject;
    using Player = ULocalPlayer;
    using Class = UClass;
    using Context = USubsystem;
    using World = UWorld;
    using FClassMap = TMap<UClass*, UBlueprintContextBase*>;

    explicit FContextManager(UGameInstance* FirstOwner) : FirstGameInstance(FirstOwner)
    {
        if (ProbeEnabled())
        {
            ObservedManager = this;
            UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS owner manager=%llx gi=%llx"), ProbeId(this), ProbeId(FirstOwner));
        }
        PostInitHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &FContextManager::OnPostWorldInitialization);
        CleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(this, &FContextManager::OnWorldCleanup);
    }
    ~FContextManager() override
    {
        if (ObservedManager == this) ObservedManager = nullptr;
        FWorldDelegates::OnPostWorldInitialization.Remove(PostInitHandle);
        FWorldDelegates::OnWorldCleanup.Remove(CleanupHandle);
    }
    void AddReferencedObjects(FReferenceCollector& Collector) override
    {
        for (auto& PlayerEntry : Players)
            for (auto& ClassEntry : PlayerEntry.Value)
                Collector.AddReferencedObject(ClassEntry.Value);
    }
    USubsystem* Lookup(ULocalPlayer* P, UClass* C) { return BlueprintContextCore::Lookup(*this, P, C); }
    bool IsLocalSubsystemClass(UClass* C) const { return C->IsChildOf(ULocalPlayerSubsystem::StaticClass()); }
    bool IsGameSubsystemClass(UClass* C) const { return C->IsChildOf(UGameInstanceSubsystem::StaticClass()); }
    USubsystem* LocalSubsystem(ULocalPlayer* P, UClass* C) const
    { return Cast<ULocalPlayerSubsystem>(P->GetSubsystemBase(TSubclassOf<ULocalPlayerSubsystem>(C))); }
    USubsystem* GameSubsystem(ULocalPlayer* P, UClass* C) const
    {
        UGameInstance* GI = P->GetGameInstance();
        return GI ? Cast<UGameInstanceSubsystem>(GI->GetSubsystemBase(TSubclassOf<UGameInstanceSubsystem>(C))) : nullptr;
    }
    USubsystem* LegacyLookup(ULocalPlayer* P, UClass* C) const
    {
        const FClassMap* Map = Players.Find(TWeakObjectPtr<ULocalPlayer>(P));
        if (!Map || !C->IsChildOf(UBlueprintContextBase::StaticClass())) return nullptr;
        UBlueprintContextBase* const* Result = Map->Find(C);
        return Result ? *Result : nullptr;
    }
    void PruneExpiredPlayers()
    { for (auto It = Players.CreateIterator(); It; ++It) if (!It.Key().IsValid()) It.RemoveCurrent(); }
    FClassMap& FindOrAddPlayer(ULocalPlayer* P) { return Players.FindOrAdd(TWeakObjectPtr<ULocalPlayer>(P)); }
    FClassMap* FindPlayer(ULocalPlayer* P) { return Players.Find(TWeakObjectPtr<ULocalPlayer>(P)); }
    void RemovePlayer(ULocalPlayer* P) { Players.Remove(TWeakObjectPtr<ULocalPlayer>(P)); if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS remove_map player=%llx remains=%d"), ProbeId(P), Players.Contains(TWeakObjectPtr<ULocalPlayer>(P)) ? 1 : 0); }
    template <typename Fn> void ForEachDerivedClass(Fn&& Function)
    {
        TArray<UClass*> Classes;
        GetDerivedClasses(UBlueprintContextBase::StaticClass(), Classes, true);
        for (UClass* C : Classes) Function(C);
    }
    bool IsAbstract(UClass* C) const { return C->HasAnyClassFlags(CLASS_Abstract); }
    void ReplaceWithNewContext(FClassMap& Map, UClass* C, ULocalPlayer* P)
    {
        UBlueprintContextBase* const* OldSlot = ProbeEnabled() ? Map.Find(C) : nullptr;
        if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS insert player=%llx class=%llx old=%llx"), ProbeId(P), ProbeId(C), ProbeId(OldSlot ? *OldSlot : nullptr));
        UBlueprintContextBase*& Slot = Map.Add(C, nullptr);
        UBlueprintContextBase* Instance = NewObject<UBlueprintContextBase>(P, C);
        Slot = Instance;
        if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS stored player=%llx class=%llx value=%llx"), ProbeId(P), ProbeId(C), ProbeId(Instance));
    }
    void ObserveInitialize(UBlueprintContextBase* C, bool Before) const
    {
        ULocalPlayer* P = Cast<ULocalPlayer>(C->GetOuter());
        const FClassMap* Map = Players.Find(TWeakObjectPtr<ULocalPlayer>(P));
        UBlueprintContextBase* const* Self = Map ? Map->Find(C->GetClass()) : nullptr;
        UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS init_%s player=%llx class=%llx object=%llx self=%llx slot=%d"), Before ? TEXT("begin") : TEXT("end"), ProbeId(P), ProbeId(C->GetClass()), ProbeId(C), ProbeId(Self ? *Self : nullptr), Self ? 1 : 0);
        if (Map && Before) for (const auto& Entry : *Map)
            UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS peer player=%llx creating=%llx class=%llx value=%llx"), ProbeId(P), ProbeId(C->GetClass()), ProbeId(Entry.Key), ProbeId(Entry.Value));
    }
    template <typename Fn> void ForEachContext(FClassMap& Map, Fn&& Function)
    { for (auto& Entry : Map) Function(Entry.Value); }
    template <typename Fn> void ForEachCopiedClassMap(Fn&& Function)
    {
        if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS copy_begin"));
        for (auto& PlayerEntry : Players)
        {
            FClassMap Copy = PlayerEntry.Value;
            if (ProbeEnabled())
            {
                UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS copy_player player=%llx"), ProbeId(PlayerEntry.Key.Get()));
                for (const auto& Entry : Copy)
                    UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS copy_expected player=%llx object=%llx"), ProbeId(PlayerEntry.Key.Get()), ProbeId(Entry.Value));
            }
            ForEachContext(Copy, Function);
            if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS copy_player_end player=%llx"), ProbeId(PlayerEntry.Key.Get()));
        }
        if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS copy_end"));
    }
    void PostInitialize(USubsystem* C) { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS post object=%llx"), ProbeId(C)); CastChecked<UBlueprintContextBase>(C)->ContextPostInitialize(); }
    void Removing(USubsystem* C, UWorld* W) { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS removing object=%llx world=%llx"), ProbeId(C), ProbeId(W)); CastChecked<UBlueprintContextBase>(C)->ContextRemoved(W); }
    void MarkPendingKill(USubsystem* C) { C->MarkPendingKill(); if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS kill object=%llx pending=%d"), ProbeId(C), C->IsPendingKill() ? 1 : 0); }
    UWorld* PlayerWorld(ULocalPlayer* P) const { return P->GetWorld(); }
    void WorldReady(USubsystem* C, UWorld* W) { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS ready object=%llx world=%llx"), ProbeId(C), ProbeId(W)); CastChecked<UBlueprintContextBase>(C)->ContextWorldInitialized(W); if (ProbeLifecycleArmed) ProbeSawReady = true; }
    void WorldCleanup(USubsystem* C, UWorld* W) { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS cleanup_dispatch object=%llx world=%llx"), ProbeId(C), ProbeId(W)); CastChecked<UBlueprintContextBase>(C)->ContextWorldCleanup(W); if (ProbeLifecycleArmed) ProbeSawCleanup = true; }
    FName WorldName(UWorld* W) const { return W->GetFName(); }
    void StorePending(FName N, UWorld* W) { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS pending world=%llx name=%s"), ProbeId(W), *N.ToString()); PendingWorlds.Add(N, W); }
    bool FirstOwnerIsValid() const { return FirstGameInstance.IsValid(); }
    void Schedule(FName N)
    { FirstGameInstance->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateRaw(this, &FContextManager::OnDeferredWorldInitialization, N)); }
    bool HasPending(FName N) const { return PendingWorlds.Contains(N); }
    UWorld* TakePending(FName N)
    {
        UWorld* W = PendingWorlds.FindChecked(N).Get();
        PendingWorlds.Remove(N);
        if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS consumed world=%llx name=%s remains=%d"), ProbeId(W), *N.ToString(), PendingWorlds.Contains(N) ? 1 : 0);
        return W;
    }
    bool WorldContextMatchesFirstOwner(UWorld* W) const
    {
        const FWorldContext* WC = GEngine->GetWorldContextFromWorld(W);
        const bool Matches = WC && WC->OwningGameInstance == FirstGameInstance.Get();
        if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS owner_match world=%llx actual=%llx first=%llx match=%d"), ProbeId(W), ProbeId(WC ? WC->OwningGameInstance : nullptr), ProbeId(FirstGameInstance.Get()), Matches ? 1 : 0);
        return Matches;
    }
private:
    void OnPostWorldInitialization(UWorld* W, const UWorld::InitializationValues)
    { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS world_init world=%llx"), ProbeId(W)); BlueprintContextCore::WorldInitialized(*this, W); }
    void OnDeferredWorldInitialization(FName N)
    { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS deferred name=%s"), *N.ToString()); BlueprintContextCore::DeferredWorldInitialized(*this, N); if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS deferred_end name=%s"), *N.ToString()); }
    void OnWorldCleanup(UWorld* W, bool, bool)
    { if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS world_cleanup world=%llx"), ProbeId(W)); BlueprintContextCore::WorldCleanup(*this, W); if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS world_cleanup_end world=%llx"), ProbeId(W)); }
    TMap<TWeakObjectPtr<ULocalPlayer>, FClassMap> Players;
    TWeakObjectPtr<UGameInstance> FirstGameInstance;
    TMap<FName, TWeakObjectPtr<UWorld>> PendingWorlds;
    FDelegateHandle PostInitHandle;
    FDelegateHandle CleanupHandle;
};

TSharedPtr<FContextManager>& ProcessManager(UGameInstance* FirstOwner)
{
    // Original140822100 registers an exit-time shared-pointer destructor1458238e0.
    static TSharedPtr<FContextManager> Instance;
    if (!Instance.IsValid()) Instance = MakeShareable(new FContextManager(FirstOwner));
    return Instance;
}

struct FContextSelection
{
    using Object = UObject;
    using Player = ULocalPlayer;
    using Class = UClass;
    using Context = USubsystem;
    bool IsWidget(UObject* O) const { return O->IsA<UUserWidget>(); }
    ULocalPlayer* WidgetPlayer(UObject* O) const { return CastChecked<UUserWidget>(O)->GetOwningLocalPlayer(); }
    bool IsController(UObject* O) const { return O->IsA<APlayerController>(); }
    ULocalPlayer* ControllerPlayer(UObject* O) const { return Cast<ULocalPlayer>(CastChecked<APlayerController>(O)->Player); }
    ULocalPlayer* AsPlayer(UObject* O) const { return Cast<ULocalPlayer>(O); }
    UWorld* WorldFromContext(UObject* O) const { return GEngine->GetWorldFromContextObject(O, EGetWorldErrorMode::ReturnNull); }
    UGameInstance* WorldGameInstance(UWorld* W) const { return W->GetGameInstance(); }
    ULocalPlayer* FirstGamePlayer(UGameInstance* GI) const { return GI->GetFirstGamePlayer(); }
    FContextManager& AcquireManager(ULocalPlayer* P) const { return *ProcessManager(P->GetGameInstance()); }
};
}

USubsystem* BlueprintContextRuntime::GetContext(UObject* ContextObject, TSubclassOf<USubsystem> Class)
{
    // The original null-context fast path precedes player selection/manager acquisition.
    // A non-null requested subsystem class is the original native caller precondition.
    FContextSelection Selection;
    return BlueprintContextCore::GetContext(Selection, ContextObject, Class.Get());
}
void BlueprintContextRuntime::CreateContexts(ULocalPlayer* Player)
{
    check(Player);
    BlueprintContextCore::Create(*ProcessManager(Player->GetGameInstance()), Player);
}
void BlueprintContextRuntime::DestroyContexts(ULocalPlayer* Player)
{
    check(Player);
    BlueprintContextCore::Destroy(*ProcessManager(Player->GetGameInstance()), Player);
}

void BlueprintContextRuntime::ObserveContextInitialize(UBlueprintContextBase* C, bool Before)
{
    if (ProbeEnabled() && ObservedManager && C) ObservedManager->ObserveInitialize(C, Before);
}

void BlueprintContextRuntime::ObserveContextDeinitialize(UBlueprintContextBase* C, bool Before)
{
    if (ProbeEnabled()) UE_LOG(LogTemp, Display, TEXT("CONTEXT_OBS deinit_%s object=%llx"), Before ? TEXT("begin") : TEXT("end"), ProbeId(C));
}
void BlueprintContextRuntime::ArmLifecycleObservation()
{
    if (ProbeEnabled()) { ProbeSawReady = false; ProbeSawCleanup = false; ProbeLifecycleArmed = true; }
}
bool BlueprintContextRuntime::LifecycleCallbacksObserved()
{
    return ProbeEnabled() && ProbeLifecycleArmed && ProbeSawReady && ProbeSawCleanup;
}
