// Incomplete rough lift; exec 0x142920370, generated/decomp/AFortGameStateAthena/OnRep_Aircraft.c.
// Types and externs below describe observed interactions, not recovered engine declarations.
#include <cstdint>
struct Aircraft; struct PlayerState; struct PawnOrComponent; struct World; struct GateActor; struct Aux;
struct Controller { PlayerState* PlayerState_at_220; Aux* Aux_at_1a40; uint8_t byte_f0; };
struct Playlist { uint8_t byte_c8; };
struct GameState {
    uint8_t GamePhase, mode_at_14a0;
    Playlist *OverridePlaylist, *BasePlaylist;
    Aircraft** Aircrafts; int32_t AircraftCount;
    World* GetWorld(); Aircraft* GetAircraft(int32_t);
};
extern bool EachGateActor(GameState*, bool (*visit)(GateActor*));
extern bool GateVirtual_b40(GateActor*);
extern void LogEmptyAircraft(GameState*), LogInvalidAircraft(GameState*), LogInvalidPlayerState(), LogInvalidTeamIndex(int32_t);
extern int32_t WorldControllerCount(World*);
extern Controller* ResolveAthenaController(World*, int32_t);
extern bool IsAthenaPlayerState(PlayerState*);
extern uint8_t PlayerTeam(PlayerState*);
extern PawnOrComponent* Call_141695900(Controller*);
extern bool Byte_f8(PawnOrComponent*), Byte_f9(PawnOrComponent*);
extern void Call_141ccfa40(PawnOrComponent*, Aircraft*);
extern void Call_1419f5840_unresolved(GameState*, Controller*, int32_t, Aircraft*); // exact arguments lost
extern void Call_1424a51b0(Aux*, int32_t);

void OnRep_Aircraft_draft(GameState* self)
{
    if (self->GamePhase != 3) return;
    if (EachGateActor(self, GateVirtual_b40)) return;
    if (self->AircraftCount == 0) { LogEmptyAircraft(self); return; }
    for (int32_t i = 0; i < self->AircraftCount; ++i)
        if (!self->Aircrafts[i]) { LogInvalidAircraft(self); return; }
    World* world = self->GetWorld();
    for (int32_t c = 0; c < WorldControllerCount(world); ++c) {
        Controller* controller = ResolveAthenaController(world, c);
        if (!controller) continue;
        for (int32_t i = 0; i < self->AircraftCount; ++i) {
            int32_t assigned = 0;
            PlayerState* ps = controller->PlayerState_at_220;
            if (!ps || !IsAthenaPlayerState(ps)) LogInvalidPlayerState();
            else {
                uint8_t team = PlayerTeam(ps);
                Playlist* playlist = self->OverridePlaylist ? self->OverridePlaylist : self->BasePlaylist;
                if (playlist && self->mode_at_14a0 == 1) {
                    assigned = int32_t(team) - int32_t(playlist->byte_c8);
                    bool invalid = assigned < 0 ||
                        ((self->GamePhase != 1 && self->GamePhase != 2) && assigned >= self->AircraftCount);
                    if (invalid) { LogInvalidTeamIndex(assigned); assigned = 0; }
                }
            }
            if (assigned == i) {
                PawnOrComponent* value = Call_141695900(controller);
                if (value && !Byte_f8(value) && !Byte_f9(value)) Call_141ccfa40(value, self->GetAircraft(i));
            }
            Aircraft* aircraft = self->GetAircraft(i);
            Call_1419f5840_unresolved(self, controller, i, aircraft);
        }
        if (controller->Aux_at_1a40 && controller->byte_f0 == 3) {
            Call_1424a51b0(controller->Aux_at_1a40, 1);
            Call_1424a51b0(controller->Aux_at_1a40, 3);
            Call_1424a51b0(controller->Aux_at_1a40, 4);
        }
    }
}
