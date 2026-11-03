// Incomplete rough lift; exec 0x142920610, body 0x1419a6d00.
// Pseudo types and externs are explicit placeholders; timer callback is unresolved.
#include <cstdint>
struct World; struct TimerManager; struct TimerHandle { uint64_t value; };
struct TimerRecord { uint8_t status; };
struct GameState {
    uint8_t LocalRole; float PlaylistEndTime; int32_t Remaining_at_11e4;
    TimerHandle CountdownHandle;
    World* GetWorld(); float ServerTime_v630();
};
extern TimerManager* WorldTimerManager(World*); // world+0x188->+0xc0 or world+0x320
extern TimerRecord* FindTimer_144621fb0(TimerManager*, TimerHandle);
extern void ClearTimer_144627760(TimerManager*, TimerHandle);
extern void BroadcastCountdownStopped(GameState*);
extern void BroadcastCountdownStarted(GameState*, int32_t);
extern float FractionExpression_unresolved(float); // observed ROUND((remaining*2)-0.5) >> 1; verify rounding semantics
extern void SetLoopingTimer_1446278d0(TimerManager*, TimerHandle*, GameState*,
                                      void (*callback)(GameState*), float rate, float firstDelay);
extern void CountdownCallback_1419a2e10(GameState*);

void OnRep_PlaylistEndTime_draft(GameState* self)
{
    if (self->LocalRole == 3) return;
    World* world = self->GetWorld();
    if (!world) return;
    TimerManager* timers = WorldTimerManager(world);
    if (self->PlaylistEndTime <= 0.0f && self->PlaylistEndTime != 0.0f) {
        TimerRecord* timer = FindTimer_144621fb0(timers, self->CountdownHandle);
        if (timer && timer->status != 2) {
            BroadcastCountdownStopped(self);
            timers = WorldTimerManager(world);
            if (FindTimer_144621fb0(timers, self->CountdownHandle))
                ClearTimer_144627760(timers, self->CountdownHandle);
            self->CountdownHandle.value = 0;
        }
        return;
    }
    float remaining = self->PlaylistEndTime - self->ServerTime_v630();
    int32_t seconds = int32_t(remaining);
    self->Remaining_at_11e4 = seconds < 1 ? 0 : seconds;
    TimerRecord* timer = FindTimer_144621fb0(timers, self->CountdownHandle);
    if (timer && timer->status != 2) return;
    BroadcastCountdownStarted(self, self->Remaining_at_11e4);
    float firstDelay = FractionExpression_unresolved(remaining);
    if (firstDelay <= 0.0001f) firstDelay = 0.0001f;
    timers = WorldTimerManager(world);
    SetLoopingTimer_1446278d0(timers, &self->CountdownHandle, self, CountdownCallback_1419a2e10, 1.0f, firstDelay);
}
