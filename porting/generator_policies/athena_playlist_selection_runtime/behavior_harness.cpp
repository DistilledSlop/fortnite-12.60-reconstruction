// Focused host contract harness; compiles the actual recovered .inc bodies.
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>
using int32 = int32_t;
using uint32 = uint32_t;
struct FString { std::string Value; const char* operator*() const { return Value.c_str(); } };
struct FName {
    int32 Index = 0, Number = 0;
    bool operator==(FName Other) const { return Index == Other.Index && Number == Other.Number; }
    bool IsNone() const { return Index == 0 && Number == 0; }
    FString ToString() const { return {std::to_string(Index) + ":" + std::to_string(Number)}; }
};
const FName NAME_None{};
struct LogRecord { std::string Level, Format; };
static std::vector<LogRecord> Logs;
#define TEXT(value) value
#define UE_LOG(category, level, format, ...) Logs.push_back({#level, format})
enum ENetMode { NM_Standalone, NM_DedicatedServer, NM_ListenServer, NM_Client };
struct UWorld { ENetMode Mode = NM_Standalone; ENetMode GetNetMode() { return Mode; } };
struct UFortPlaylistAthena { int32 PlaylistId; FName PlaylistName; };
struct UFortPlaylistManager {
    std::vector<UFortPlaylistAthena*> AthenaPlaylists;
    UFortPlaylistAthena* GetPlaylistByIdNative(int32 PlaylistId);
    UFortPlaylistAthena* GetPlaylistByNameNative(FName PlaylistName);
};
struct UGameInstance {};
struct UFortGameInstance : UGameInstance { UFortPlaylistManager* PlaylistManager; };
struct AFortGameMode {
    int32 CurrentPlaylistId = -77;
    FName CurrentPlaylistName{77, 0};
    UWorld World;
    UGameInstance* Instance = nullptr;
    UWorld* GetWorld() { return &World; }
    UGameInstance* GetGameInstance() { return Instance; }
    virtual void SetCurrentPlaylistId(int32 PlaylistId);
    virtual void SetCurrentPlaylistName(FName PlaylistName);
    virtual void NativeSetCurrentPlaylistName(FName PlaylistName);
    UFortPlaylistManager* GetPlaylistManagerNative();
};
struct AFortGameModeAthena : AFortGameMode {
    void SetCurrentPlaylistId(int32 PlaylistId) override;
    void SetCurrentPlaylistName(FName PlaylistName) override;
};

UFortPlaylistManager* AFortGameMode::GetPlaylistManagerNative()
{
#include "AFortGameMode.GetPlaylistManagerNative.inc"
}

void AFortGameMode::SetCurrentPlaylistId(int32 PlaylistId)
{
#include "AFortGameMode.SetCurrentPlaylistId.inc"
}

void AFortGameMode::SetCurrentPlaylistName(FName PlaylistName)
{
#include "AFortGameMode.SetCurrentPlaylistName.inc"
}

void AFortGameMode::NativeSetCurrentPlaylistName(FName PlaylistName)
{
#include "AFortGameMode.NativeSetCurrentPlaylistName.inc"
}

void AFortGameModeAthena::SetCurrentPlaylistId(int32 PlaylistId)
{
#include "AFortGameModeAthena.SetCurrentPlaylistId.inc"
}

void AFortGameModeAthena::SetCurrentPlaylistName(FName PlaylistName)
{
#include "AFortGameModeAthena.SetCurrentPlaylistName.inc"
}

UFortPlaylistAthena* UFortPlaylistManager::GetPlaylistByIdNative(int32 PlaylistId)
{
#include "UFortPlaylistManager.GetPlaylistByIdNative.inc"
}

UFortPlaylistAthena* UFortPlaylistManager::GetPlaylistByNameNative(FName PlaylistName)
{
#include "UFortPlaylistManager.GetPlaylistByNameNative.inc"
}

struct DispatchProbe : AFortGameModeAthena {
    int Calls = 0;
    void NativeSetCurrentPlaylistName(FName) override { ++Calls; }
};
int main()
{
    AFortGameMode Base;
    assert(Base.GetPlaylistManagerNative() == nullptr);
    for (int32 Id : {0, 1}) { Base.SetCurrentPlaylistId(Id); assert(Base.CurrentPlaylistId == Id); assert(Base.CurrentPlaylistName.IsNone()); }
    for (int32 Id : {-1, 2, INT32_MIN, INT32_MAX}) { Base.SetCurrentPlaylistId(Id); assert(Base.CurrentPlaylistId == 1); }
    Logs.clear(); Base.SetCurrentPlaylistName({10, 0}); assert(Logs.empty());
    Base.World.Mode = NM_Client; Base.SetCurrentPlaylistName({10, 0});
    assert(Logs.size() == 1 && Logs[0].Level == "Warning"); assert(Base.CurrentPlaylistId == 1);
    UFortPlaylistAthena First{2, {10, 0}}, DuplicateId{2, {11, 0}}, Numbered{3, {10, 1}};
    UFortPlaylistManager Manager{{nullptr, &First, &DuplicateId, &Numbered}};
    assert(Manager.GetPlaylistByIdNative(2) == &First);
    assert(Manager.GetPlaylistByNameNative({10, 1}) == &Numbered);
    assert(Manager.GetPlaylistByNameNative({10, 0}) == &First);
    Logs.clear(); assert(Manager.GetPlaylistByNameNative(NAME_None) == nullptr); assert(Logs.empty());
    assert(Manager.GetPlaylistByNameNative({0, 1}) == nullptr); assert(Logs.size() == 1);
    UFortGameInstance Instance; Instance.PlaylistManager = &Manager;
    AFortGameModeAthena Athena; Athena.Instance = &Instance;
    AFortGameMode* Virtual = &Athena;
    Logs.clear(); Virtual->SetCurrentPlaylistId(2);
    assert(Athena.CurrentPlaylistId == 2 && Athena.CurrentPlaylistName == First.PlaylistName);
    assert(Logs.size() == 2 && Logs[0].Level == "Warning" && Logs[1].Level == "Log");
    Virtual->SetCurrentPlaylistName({10, 1}); assert(Athena.CurrentPlaylistId == 3);
    Logs.clear(); Virtual->SetCurrentPlaylistName({90, 0});
    assert(Athena.CurrentPlaylistId == 3 && Athena.CurrentPlaylistName == Numbered.PlaylistName);
    assert(Logs.size() == 2 && Logs[0].Level == "Error" && Logs[1].Level == "Error");
    Logs.clear(); Virtual->SetCurrentPlaylistName(NAME_None); assert(Logs.size() == 1);
    Logs.clear(); Virtual->SetCurrentPlaylistId(-3); assert(Athena.CurrentPlaylistId == 3 && Logs.size() == 2);
    DispatchProbe Probe; Virtual = &Probe; Virtual->SetCurrentPlaylistName({10, 0}); assert(Probe.Calls == 1);
}
