// Concrete bindings for native 0x142081fa0 (AFortMinigame::AddMinigamePlayer(AFortPlayerState*, bool bForced)).
// Evidence: reflection json (AFortMinigame class block 0x146446ef0 property offsets), generated headers, and the
// native decompilation. Items under "UNRESOLVED" are declarations only; they are NOT implemented.
#pragma once

// ---- Offset guards (reflected, ascending-offset layout; bool offsets inferred by packing after FRotator@0x50) ----
static_assert(STRUCT_OFFSET(FMinigamePlayer, UniquePlayerId) == 0x10, "record identity");   // native lVar12+0x10
static_assert(STRUCT_OFFSET(FMinigamePlayer, ClassSlotIndex) == 0x39, "record +0x39");      // reflected 57
static_assert(STRUCT_OFFSET(FMinigamePlayer, TeamBeforeMinigameStarted) == 0x40, "record +0x40");
static_assert(STRUCT_OFFSET(FMinigamePlayer, LocationBeforeMinigameStarted) == 0x44, "record +0x44");
static_assert(STRUCT_OFFSET(FMinigamePlayer, RotationBeforeMinigameStarted) == 0x50, "record +0x50");
static_assert(STRUCT_OFFSET(FMinigamePlayer, SolPlayer) == 0x70, "record +0x70");
// bHasValidResetData=+0x38, bIsLocationBeforeGameStartedBlocked=+0x5c, bIsTeleportingOrRespawningForGameplay=+0x5f
// are not reflected with offsets; they are the unique packed positions (UniqueId ends 0x38; rotator ends 0x5c).
// NOTE: README labels (+0x38 "active", +0x5c "initialized", +0x5f "spectating") are NOT reflected names; use fields.
// Record +0x78 (8 bytes) and +0x80 (int32 slot) are beyond the reflected struct and have no reflected member.
static inline uint8*  Minigame94_RecordRaw(FMinigamePlayer& R, int32 Off) { return reinterpret_cast<uint8*>(&R) + Off; }

// ---- AFortMinigame field bindings (reflected) ----
//  +0x438 PlayerArray, +0x558 SpectatingPlayerArray (FMinigamePlayers), +0x348 OnPlayerJoinInProgress (FOnPlayerJoinInProgress(APawn*)),
//  +0x870 CurrentState (EFortMinigameState), +0x910 Volume (AFortVolume*), +0xaa5 PlayerPersistence (EMinigamePlayerPersistence),
//  +0xb80 ClientMinigameStartedText (FText; native +0xb88 is its shared-reference controller), +0xc80 PlayerBuckets.
// CORRECTION to the old proposal: +0xaa5 is PlayerPersistence, not a spawn-location policy; native values 0/1/2 map to
// EMinigamePlayerPersistence enumerators (names to be confirmed from EMinigamePlayerPersistence.h by root).
// NOT reflected (raw only): Mode int32 @+0xb10, PreferredTeam uint8 @+0xb14. Bound as raw reads, nothing guessed about meaning.
static inline int32 Minigame94_Mode(const AFortMinigame* G)          { return *reinterpret_cast<const int32*>(reinterpret_cast<const uint8*>(G) + 0xb10); }
static inline uint8 Minigame94_PreferredTeamRaw(const AFortMinigame* G) { return *(reinterpret_cast<const uint8*>(G) + 0xb14); }

static inline bool Minigame94_IsAuthority(const AActor* A) { return A->GetLocalRole() == ROLE_Authority; } // byte +0xf0 == 3

// Player state -> controller: AActor::Owner (+0xe0) cast with IsChildOf(AFortPlayerControllerAthena).
static inline AFortPlayerControllerAthena* Minigame94_AthenaController(AFortPlayerState* P)
{ return Cast<AFortPlayerControllerAthena>(P->GetOwner()); }
static inline bool Minigame94_IsAthenaPlayerState(const AFortPlayerState* P) { return P->IsA(AFortPlayerStateAthena::StaticClass()); }

static inline const FText& Minigame94_JoinTooLateText()
{ static const FText T = NSLOCTEXT("FortMinigame", "JoinInProgressTooLate", "The game you were tring to join had already ended."); return T; }

// Bucket checks: native compares (int)uint8 team < PlayerBuckets.Num() (Game+0xc88 count).
static inline bool Minigame94_IsBucketValid(const AFortMinigame* G, uint8 T) { return (int32)T < G->PlayerBuckets.Num(); }
// Spectating-free join: PlayerBuckets[T].PlayerIds.Add(Player identity). Native copy-constructs a 0x28 FUniqueNetIdRepl from
// Player+0x248 (copies the shared-ref identity, bumps shared count, leaves the replication-data array empty).
static inline void Minigame94_AppendPlayerIdentityToBucket(AFortMinigame* G, uint8 T, AFortPlayerState* P)
{ G->PlayerBuckets[T].PlayerIds.Add(P->GetUniqueId()); }
// Team passed to the assignment call is PlayerBuckets[Index].TeamIdAtRoundStart (byte +1 of the 0x18 element), NOT the index itself.
static inline uint8 Minigame94_BucketRoundStartTeam(const AFortMinigame* G, int32 I) { return G->PlayerBuckets[I].TeamIdAtRoundStart; }

// Record writes (reflected fields; see offset notes above).
static inline void Minigame94_SetRecordFlagsAndLocation(FMinigamePlayer& R, uint8 PlayerTeamValue, APawn* Pawn)
{
    R.bHasValidResetData = true;                        // +0x38
    R.TeamBeforeMinigameStarted = PlayerTeamValue;      // +0x40 (value from Player+0x318 vtable+0x28; resolved by caller)
    if (Pawn)
    {   // native reads RootComponent (+0x130) world translation / rotation (cached quat->rotator at +0x180/+0x190);
        // this is AActor::GetActorLocation/Rotation. Zero-root fallback constants (_DAT_148218630/640) not read here.
        R.LocationBeforeMinigameStarted = Pawn->GetActorLocation(); // +0x44..0x4f
        R.RotationBeforeMinigameStarted = Pawn->GetActorRotation(); // +0x50..0x5b
    }
    R.bIsLocationBeforeGameStartedBlocked = true;       // +0x5c
}

// Delegate bindings (both target UFUNCTIONs present in AFortMinigame's reflected function list).
//  AddDynamic form: native compacts the invocation list, then appends {Game, FName} without uniqueness check.
static inline void Minigame94_BindTeamIndexChanged(AFortPlayerStateAthena* P, AFortMinigame* G)
{ P->OnTeamIndexChangedDelegate.AddDynamic(G, &AFortMinigame::OnPlayerStateTeamIndexChanged); }   // P+0xd90, name "OnPlayerStateTeamIndexChanged"
static inline void Minigame94_BindBeginSpectating(AFortPlayerStateZone* P, AFortMinigame* G)
{ P->OnBeginSpectatingEvent.AddDynamic(G, &AFortMinigame::OnBeginSpectating); }                    // P+0xb48, name "OnBeginSpectating"
static inline void Minigame94_BroadcastJoinInProgress(AFortMinigame* G, APawn* Pawn)
{ G->OnPlayerJoinInProgress.Broadcast(Pawn); } // native FUN_1406a9dc0(Game+0x348, &Pawn); delegate signature takes APawn*

// Native copy+release of ClientMinigameStartedText ref (Game+0xb88 when non-null) when !bForced, in the playing path.
static inline void Minigame94_TouchOptionalSharedState(AFortMinigame* G) { FText Copy = G->ClientMinigameStartedText; (void)Copy; }

// Fast-array dirty: native 0x140719c10(Array, Record) == FFastArraySerializer::MarkItemDirty.
static inline void Minigame94_MarkArrayItemDirty(FMinigamePlayers& A, FMinigamePlayer& R) { A.MarkItemDirty(R); }
// Identity assignment: temp = 0x142419ef0(Player) (0x28 FUniqueNetIdRepl); Record.UniquePlayerId = temp (0x140d7c4c0); temp destroyed.
static inline void Minigame94_CopyPlayerIdentityToRecord(FMinigamePlayer& R, const FUniqueNetIdRepl& Identity) { R.UniquePlayerId = Identity; }

// ---- UNRESOLVED native helpers (declarations only; not implemented) ----
FMinigamePlayer*   Minigame94_FindOrCreatePlayerRecord(FMinigamePlayers& A, const FUniqueNetIdRepl& Id); // 0x1420898c0 (called with Player+0x248)
void*              Minigame94_IsRegistered(FMinigamePlayer& R, UObject* WorldContext, const TCHAR* Caller); // 0x1420935b0 (non-null == registered)
UObject*           Minigame94_GetWorldContext(AFortMinigame* G);   // Game vtable +0x150
APawn*             Minigame94_PlayerPawn(AFortPlayerState* P);     // 0x1418563c0
FUniqueNetIdRepl   Minigame94_MakePlayerIdentity(AFortPlayerState* P); // 0x142419ef0
bool               Minigame94_RejectLateJoinGlobalEnabled();       // global _DAT_1478c5050 != 0
bool               Minigame94_RejectLateJoin(AFortMinigame* G);    // 0x1420997a0
void               Minigame94_SendControllerMessage(AFortPlayerControllerAthena* C, const FText& T); // Controller vtable +0xa58
bool               Minigame94_ManagerAllowsImmediateJoin(AFortMinigame* G); // 0x14297b320 key, Volume(+0x910) vtable +0x610 -> result byte +0x23a
void               Minigame94_EnsurePlayerSubobjectState(AFortPlayerState* P); // 0x142ec0440
ASOLFortPlayer*    Minigame94_ResolveSolPlayer(AFortPlayerState* P);  // 0x142b8c150 then 0x142ed7360(.., P, 0, 0) -> Record.SolPlayer (+0x70)
void               Minigame94_TrySetRecordSlotFromController(AFortMinigame* G, UObject* Ctx, AFortPlayerState* P, FMinigamePlayer& R); // gates in README; 0x1428ea7f0,0x140e2d090,0x141c8cc90,0x141ffa200; writes +0x78/+0x80
uint8              Minigame94_PlayerTeamValue(AFortPlayerState* P);   // Player+0x318 vtable +0x28
void               Minigame94_BeginSpectatingState(AFortPlayerState* P);   // 0x141a3d510
void               Minigame94_PawnSpectatorAction(APawn* Pawn);            // Pawn vtable +0xa80(arg *(0x141ed1800()+0x448),0,0)
void               Minigame94_RefreshSpectatorController(AFortPlayerControllerAthena* C); // 0x142a6e250
int32              Minigame94_ChooseTeam(AFortMinigame* G, AFortPlayerState* P);  // 0x142083810
void               Minigame94_AssignPlayerTeam(AFortMinigame* G, AFortPlayerState* P, uint8 Team, bool bFlag); // 0x1420a5820
void               Minigame94_ClassSlotDefault(AFortMinigame* G, AFortPlayerState* P); // 0x142083e10 when Record.ClassSlotIndex == 0xff
void               Minigame94_CompletePlayerSetup(AFortMinigame* G, TWeakObjectPtr<AFortPlayerState> P); // 0x142089b10
bool               Minigame94_ControllerDefersPlayerSetup(AFortPlayerControllerAthena* C); // Controller byte +0x2e78
void               Minigame94_ScheduleWeakPlayerSetup(AFortMinigame* G, UObject* Ctx, TWeakObjectPtr<AFortPlayerState> P); // delegate vtables 0x1459cb3b8/0x1459cb410, 0x142bad740, 0x144627d90 on Ctx timer owner (+0x188 -> +0xc0, else +0x320)
void               Minigame94_SetControllerMinigame(AFortPlayerControllerAthena* C, AFortMinigame* G); // C vtable +0x1be0(2)->+0x30 = Game
void               Minigame94_NotifyPlayerWithoutController(AFortMinigame* G, AFortPlayerState* P);   // 0x1420a7410
void               Minigame94_LogAddMinigamePlayer(int32 Verbosity, const TCHAR* Fmt, AFortMinigame* G, AFortPlayerState* P); // category 0x14813df34; game name = G+0x18 FName::ToString via 0x142ce9f90; player string 0x1444ac7a0 (not GetName-confirmed)
