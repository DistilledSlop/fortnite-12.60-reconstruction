// Shared bindings for minigame_large96_runtime (source draft, uncounted, build pending, nothing compiled/tested).
// Reuses ../minigame_large94_runtime/AddMinigamePlayer.bindings.inc unchanged (IsAuthority, AthenaController, PlayerTeamValue,
// MakePlayerIdentity, RejectLateJoin 0x1420997a0, AppendPlayerIdentityToBucket, ...). Only NEW pieces are declared here.
#pragma once
#include "../minigame_large94_runtime/AddMinigamePlayer.bindings.inc"

// ---- Reflected AFortMinigame offsets used by the four 96 functions (reflection json / bool_bits.json) ----
//  +0x308 PlayerChangedTeamDelegate (FOnPlayerChangedTeam(AFortPlayerState*, uint8 TeamId, uint8 OldTeamId, bool bTeamChangedWithoutRespawn))
//  +0x438 PlayerArray, +0x698 PlayerDeathsStatFilter, +0x798 AutoStartDelay (int32), +0x7a0 WarmupDuration (float),
//  +0x7c0 TeamArray (FMinigameTeam, stride 0x28; TeamIndex +0, bHasBucketAvailable +0x24), +0x870 CurrentState,
//  +0x910 Volume, +0x91a bTeamMinigame (bool_bits), +0xaa0 NumTeams (int32), +0xaa8 MinigameStarter, +0xc80 PlayerBuckets (stride 0x18).
// Unreflected raw members: PlayerArray.Players.Num() read at Game+0x548 (FFastArraySerializer tail: Players Data +0x540, Num +0x548);
//  Game+0x8b0 float (between ServerTime +0x8a8 and CachedTime +0x8bc), Game+0xb38 FTimerHandle (countdown), Game+0x228 (sub-object, vtbl+0x10).
template <typename T> static inline T& Minigame96_Raw(void* Base, int32 Off) { return *reinterpret_cast<T*>(reinterpret_cast<uint8*>(Base) + Off); }
static inline int32 Minigame96_PlayerArrayNum(AFortMinigame* G) { return G->PlayerArray.Players.Num(); } // == raw int32 @ +0x548
static inline float& Minigame96_CountdownEndTime(AFortMinigame* G) { return Minigame96_Raw<float>(G, 0x8b0); }
static inline FTimerHandle& Minigame96_CountdownTimer(AFortMinigame* G) { return Minigame96_Raw<FTimerHandle>(G, 0xb38); }
static inline int32& Minigame96_AutoStartDelay(AFortMinigame* G) { return G->AutoStartDelay; }

// FMinigamePlayer tail beyond the reflected struct (0x78 8 bytes, 0x80 int32): FTimerHandle used by DelayedTeleportPlayer is at +0x88.
static inline FTimerHandle& Minigame96_RecordTeleportTimer(FMinigamePlayer& R) { return *reinterpret_cast<FTimerHandle*>(reinterpret_cast<uint8*>(&R) + 0x88); }

// PlayerState raw members: +0xd30 byte (zero => "no longer needs to respawn"; reflected name not found), +0x248 UniqueId.
static inline uint8 Minigame96_PlayerNeedsRespawnRaw(AFortPlayerState* P) { return Minigame96_Raw<uint8>(P, 0xd30); }

// ---- Logging (category 0x14813df34, runtime verbosity byte DAT_14813df30; gate is `N < verbosity`, i.e. level >= N+1) ----
bool Minigame96_LogEnabled(int32 VerbosityGate);                                 // reads DAT_14813df30 (native: `gate < DAT_14813df30`)
void Minigame96_Logf(int32 Level, const TCHAR* Fmt, ...);                         // FMsg::Logf_Internal(0,0,category 0x14813df34,Level,Fmt,...)
FString Minigame96_ObjectLogString(const UObject* Obj);                           // 0x142ce9f90 (native object name string)
static inline FString Minigame96_PlayerIdString(AFortPlayerState* P) { return P->GetUniqueId().ToString(); } // 0x140b35480(Player+0x248)

// ---- UNRESOLVED native helpers introduced by the 96 batch (declarations only) ----
// 0x142083810 AssignPlayerToAvailableBucket
bool    Minigame96_Query1420a6bc0(AFortMinigame* G);                 // arg elided by decompiler; bool gate "use custom teams"
bool    Minigame96_Query142700740();                                  // called after 0x1429da540 + Game+0x228 sub-object vtbl+0x10/+0x610 (+0xb0 != 0)
void    Minigame96_Prep1429da540();                                   // no visible args
UObject* Minigame96_SubObject228Resolve(AFortMinigame* G);            // (*(vtbl(Game+0x228)+0x10))()
UObject* Minigame96_ResolveVtbl610(UObject* O);                       // (*(vtbl(O)+0x610))(O); result checked +0xb0 != 0
int32   Minigame96_AddBucket(AFortMinigame* G, bool bFlag);           // 0x1420830a0(this,1) -> new bucket index
void    Minigame96_MoveToSpectate(AFortMinigame* G, AFortPlayerState* P); // 0x14209b5a0
void    Minigame96_RefreshBuckets(AFortMinigame* G);                  // 0x1420a9a30
// 0x142089b10 DelayedTeleportPlayer
FMinigamePlayer* Minigame96_FindPlayerRecord(FMinigamePlayers& A, const FUniqueNetIdRepl& Id); // 0x14208d7e0(Game+0x438, &Player->UniqueId)
UObject* Minigame96_VolumeLookup(AFortVolume* V);                     // 0x141c74820(Game+0x910)
bool    Minigame96_VolumeContains(UObject* VolumeLookupResult, AFortPlayerState* P); // 0x141c94c20
void    Minigame96_PlayerStateSetFlag(AFortPlayerState* P, bool b);   // 0x141a5b590(P,1)
bool    Minigame96_PlayerStateQuery(AFortPlayerState* P);             // 0x141a4b2b0(P): true => stop
// 0x1420a5820 SetTeamForPlayer
void    Minigame96_RefreshState(AFortMinigame* G);                    // 0x142087770
UObject* Minigame96_WorldAux(UWorld* W);                              // 0x141641350(world)
bool    Minigame96_GetPlaylistTeams(UObject* WorldAux, TArray<uint8>& OutTeams); // 0x141996680 (args elided; fills byte array)
AGameModeBase* Minigame96_WorldGameMode(UWorld* W);                   // 0x141641300(world)  (== UWorld::GetAuthGameMode, unproven)
AController* Minigame96_PlayerController(AFortPlayerState* P);        // 0x142414c60
bool    Minigame96_GameModeSetTeam(AGameModeBase* GM, AController* C, uint8 Team); // GM vtbl+0xb28(GM, ctrl, team)
FString Minigame96_ControllerLogName(AController* C);                 // Controller+0x590 object vtbl+0x28 then 0x140b35480 string
void    Minigame96_ResetPlayerStat(AFortMinigame* G, AFortPlayerState* P, TSubclassOf<UFortMinigameStatFilter> F, int32 V); // 0x1420a4190
void    Minigame96_RemoveIdentityFromIds(TArray<FUniqueNetIdRepl>& Ids, const FUniqueNetIdRepl& Id); // 0x141e89b00(&Ids,&Player->UniqueId)
// 0x1420a7410 StartMinigameCountdown
bool    Minigame96_Query142099810(AFortMinigame* G);                  // 0x142099810(this)
float   Minigame96_GlobalDefaultCountdown();                          // _DAT_1478c5014
void    Minigame96_StartCountdownNow(AFortMinigame* G, AFortPlayerControllerAthena* C); // 0x1429bb7f0(this, ctrl)
int32 Minigame96_GlobalFlag1478c5044();                        // _DAT_1478c5044 (== 1 gate in 0x142083810)

// Unresolved weak callback factory and body, native instance vtable0x14608f9c0.
// Do not substitute the direct-start call until the callback invocation is recovered.
FTimerDelegate Minigame96_BindCountdownCallback(AFortMinigame* Game, TWeakObjectPtr<AFortPlayerControllerAthena> Controller);
