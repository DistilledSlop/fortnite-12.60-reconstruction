// UNCOUNTED rough source. Not a build input. U_* declarations are unresolved
// native effects, never fallback implementations. Source names are provisional.
// FString/FText/FFormatNamedArguments express the apparent UE value semantics;
// exact overloads, copied text flags and temporary destruction must be closed
// before promotion. See HANDOFF.md for source windows and ordering constraints.
namespace playerstate_messages69 {
struct PlayerState;
struct World;
struct GameState;
struct Controller;
struct Globals;
struct Service;

extern World* U_GetWorld_150(PlayerState*);
extern GameState* U_GameState130_CastAthena(World*);
extern Service* U_14463f750_Global148351eb8(World*);
extern Controller* U_14447f220_CastAthena(Service*, World*);
extern bool U_14241bc10(World*, PlayerState*);
extern FString U_1444ac7a0_PlayerName(PlayerState*);
extern FString U_1423d5330_PlayerName(Controller*, PlayerState*);
extern FString U_NameVirtual830(PlayerState*);
extern uint8 U_RoleF0(PlayerState*);
extern int32 U_SeasonLevelUIDisplay(PlayerState*);
// Native printf decompilation loses the integer variadic argument. Do not
// silently assume it equals the positive level used in the branch condition.
extern FString U_142bafb80_UnresolvedPrintfLevel(PlayerState*, const FString&);
extern FText U_142c26f20_FromString(const FString&);
extern FText U_142bedc40_Localized(const TCHAR* Source, const TCHAR* Namespace,
                                 const TCHAR* Key);
extern FText U_142c0c460_CopyTemplate(const FText&);
extern FText U_142c255f0_Format(const FText&, const FFormatNamedArguments&);
extern void U_GameStateVirtualA68(GameState*, const FText&, uint8);
extern Globals* U_141ed1800();
extern FString U_143e8b3b0_SoftReference24c8(Globals*);
extern Service* U_141e5b2a0();
extern FText U_141e5f780_ResolveTemplate(Service*, const FString&);
// AsNumber-like native call: int NewLevel, null options, default culture
// initialized as a zero shared-pointer pair. Exact number-format API unresolved.
extern FText U_142c18a00_Number(int32 NewLevel);
extern bool U_142c2e440_TextPredicate(const FText&);
extern GameState* U_GameState130_NoCast(World*);
extern bool U_141a2d0f0(GameState*);

static Controller* FindController(World* WorldPtr)
{
    Service* ServicePtr = U_14463f750_Global148351eb8(WorldPtr);
    return ServicePtr ? U_14447f220_CastAthena(ServicePtr, WorldPtr) : nullptr;
}

// OnRep_ThankedBusDriver / OnRep_DidNotThankBusDriver share the visible
// algorithm. No thanked/not-thanked boolean guard exists in either native body.
static void BusDriverMessage(PlayerState* Self, const TCHAR* Source,
                             const TCHAR* Key)
{
    World* WorldPtr = U_GetWorld_150(Self);
    if (!WorldPtr) return;
    GameState* State = U_GameState130_CastAthena(WorldPtr);
    if (!State) return;
    Controller* LocalController = FindController(WorldPtr);
    FString Style(TEXT("Victim"));
    if (U_14241bc10(WorldPtr, Self)) Style = TEXT("TeamVictim");
    FString Name = LocalController
        ? U_1423d5330_PlayerName(LocalController, Self)
        : U_1444ac7a0_PlayerName(Self);
    if (U_SeasonLevelUIDisplay(Self) > 0)
        Name = U_142bafb80_UnresolvedPrintfLevel(Self, Name); // "%s (%d)"
    FText Message; // native default text constructed before named arguments
    FFormatNamedArguments Arguments;
    Arguments.Add(TEXT("VictimStyle"), U_142c26f20_FromString(Style));
    Arguments.Add(TEXT("PlayerName"), U_142c26f20_FromString(Name));
    Message = U_142c255f0_Format(U_142c0c460_CopyTemplate(
        U_142bedc40_Localized(Source, TEXT("Athena"), Key)), Arguments);
    U_GameStateVirtualA68(State, Message, 0);
    // Native destroys argument map before Message, Name and Style. This scope
    // has that order, but intermediate FText reference counts remain unproved.
}

void OnRep_ThankedBusDriver(PlayerState* Self)
{
    BusDriverMessage(Self,
        TEXT("<{VictimStyle}>{PlayerName}</> has thanked the bus driver"),
        TEXT("ThankedBusDriver"));
}

void OnRep_DidNotThankBusDriver(PlayerState* Self)
{
    BusDriverMessage(Self,
        TEXT("<{VictimStyle}>{PlayerName}</> didn't thank the bus driver"),
        TEXT("DidNotThankBusDriver2"));
}

// exec 142a8dc30; inline native continuation ends 141a52d0d.
void OnRep_RebootCounter(PlayerState* Self)
{
    if (U_RoleF0(Self) == 3) return;
    World* WorldPtr = U_GetWorld_150(Self);
    if (!WorldPtr) return;
    GameState* State = U_GameState130_CastAthena(WorldPtr);
    if (!State) return;
    Controller* LocalController = FindController(WorldPtr);
    FFormatNamedArguments Arguments; // constructed before Style in this body
    FString Style(TEXT("Victim"));
    if (U_14241bc10(WorldPtr, Self)) Style = TEXT("TeamVictim");
    Arguments.Add(TEXT("VictimStyle"), U_142c26f20_FromString(Style));
    FString Name = LocalController
        ? U_1423d5330_PlayerName(LocalController, Self)
        : U_1444ac7a0_PlayerName(Self);
    Arguments.Add(TEXT("PlayerName"), U_142c26f20_FromString(Name));
    const FText Message = U_142c255f0_Format(U_142c0c460_CopyTemplate(
        U_142bedc40_Localized(
            TEXT("<{VictimStyle}>{PlayerName}</> was rebooted"),
            TEXT("Athena"), TEXT("RebootedPlayerName"))), Arguments);
    U_GameStateVirtualA68(State, Message, 0);
    // No season-level suffix in reboot path. Native destroys Message, Name,
    // Style, then map; retain separate ordering from bus-driver paths.
}

// Actual virtual implementation 141a3d950, not the reflected RPC exec wrapper.
void Client_OnNewLevel_Implementation(PlayerState* Self, int32 NewLevel)
{
    if (NewLevel <= 0) return;
    FString Style(TEXT("Victim"));
    FString Name = U_NameVirtual830(Self);
    FText Message;
    FFormatNamedArguments Arguments;
    Arguments.Add(TEXT("VictimStyle"), U_142c26f20_FromString(Style));
    Arguments.Add(TEXT("PlayerName"), U_142c26f20_FromString(Name));
    Arguments.Add(TEXT("Level"), U_142c18a00_Number(NewLevel));
    {
        Globals* GlobalsPtr = U_141ed1800(); // native has no null guard
        const FString Reference = U_143e8b3b0_SoftReference24c8(GlobalsPtr);
        Service* ServicePtr = U_141e5b2a0();
        Message = U_142c255f0_Format(U_142c0c460_CopyTemplate(
            U_141e5f780_ResolveTemplate(ServicePtr, Reference)), Arguments);
    } // native releases reference/template temporaries before evaluating text
    if (!U_142c2e440_TextPredicate(Message)) {
        World* WorldPtr = U_GetWorld_150(Self); // deliberately deferred
        if (WorldPtr) {
            GameState* State = U_GameState130_NoCast(WorldPtr);
            if (State && U_141a2d0f0(State))
                U_GameStateVirtualA68(State, Message, 0);
        }
    }
}
} // namespace playerstate_messages69
