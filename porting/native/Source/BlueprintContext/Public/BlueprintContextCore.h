#pragma once

// These algorithms are shared by the UE adapter and the portable contract tests.
// Host owns storage, engine calls, weak references and native virtual dispatch.
namespace BlueprintContextCore
{
    template <typename SuperPhase, typename IsCDO, typename Phase>
    void InstancePhase(SuperPhase Super, IsCDO IsClassDefaultObject, Phase Instance)
    {
        Super();
        if (!IsClassDefaultObject()) Instance();
    }

    template <typename Host>
    typename Host::Context* Lookup(Host& H, typename Host::Player* Player, typename Host::Class* Class)
    {
        if (H.IsLocalSubsystemClass(Class))
            if (auto* Result = H.LocalSubsystem(Player, Class)) return Result;
        if (H.IsGameSubsystemClass(Class))
            if (auto* Result = H.GameSubsystem(Player, Class)) return Result;
        return H.LegacyLookup(Player, Class);
    }

    template <typename Host>
    typename Host::Context* GetContext(Host& H, typename Host::Object* Object, typename Host::Class* Class)
    {
        if (!Object) return nullptr;
        typename Host::Player* Player = nullptr;
        if (H.IsWidget(Object)) Player = H.WidgetPlayer(Object);
        else if (H.IsController(Object)) Player = H.ControllerPlayer(Object);
        else Player = H.AsPlayer(Object);
        if (!Player)
        {
            auto* World = H.WorldFromContext(Object);
            if (!World) return nullptr;
            auto* GI = H.WorldGameInstance(World);
            if (!GI) return nullptr;
            Player = H.FirstGamePlayer(GI);
            if (!Player) return nullptr;
        }
        // Acquiring the process manager occurs even for the subsystem branches.
        auto& Manager = H.AcquireManager(Player);
        return Manager.Lookup(Player, Class);
    }

    template <typename Host>
    void Create(Host& H, typename Host::Player* Player)
    {
        H.PruneExpiredPlayers();
        auto& Map = H.FindOrAddPlayer(Player);
        H.ForEachDerivedClass([&](typename Host::Class* Class)
        {
            if (!H.IsAbstract(Class)) H.ReplaceWithNewContext(Map, Class, Player);
        });
        H.ForEachContext(Map, [&](typename Host::Context* Context) { H.PostInitialize(Context); });
    }

    template <typename Host>
    void Destroy(Host& H, typename Host::Player* Player)
    {
        auto* World = H.PlayerWorld(Player);
        auto* Map = H.FindPlayer(Player);
        if (!Map) return;
        H.ForEachContext(*Map, [&](typename Host::Context* Context)
        {
            H.Removing(Context, World);
            H.MarkPendingKill(Context);
        });
        H.RemovePlayer(Player);
    }

    template <typename Host>
    void WorldInitialized(Host& H, typename Host::World* World)
    {
        auto Name = H.WorldName(World);
        H.StorePending(Name, World);
        if (H.FirstOwnerIsValid()) H.Schedule(Name);
    }

    template <typename Host, typename Name>
    void DeferredWorldInitialized(Host& H, Name NameValue)
    {
        if (!H.HasPending(NameValue)) return;
        auto* World = H.TakePending(NameValue); // Remove before ownership lookup/dispatch.
        if (H.WorldContextMatchesFirstOwner(World))
            H.ForEachCopiedClassMap([&](typename Host::Context* Context) { H.WorldReady(Context, World); });
    }

    template <typename Host>
    void WorldCleanup(Host& H, typename Host::World* World)
    {
        if (H.WorldContextMatchesFirstOwner(World))
            H.ForEachCopiedClassMap([&](typename Host::Context* Context) { H.WorldCleanup(Context, World); });
    }
}
