// Connected rough lift of 0x1419f5920 and 0x1419fb550. Not an ABI declaration.
#include <cstdint>
#include <cmath>
#include <cstring>

namespace native74 {
using P = void*;
using U64 = std::uint64_t;
template<class T> T rd(P p, std::size_t o) { T v; std::memcpy(&v, static_cast<char*>(p)+o, sizeof v); return v; }
template<class T> void wr(P p, std::size_t o, T v) { std::memcpy(static_cast<char*>(p)+o, &v, sizeof v); }
struct VecBits { U64 xy; std::uint32_t z; };

// Every declaration below is an unresolved engine/ABI adapter, not a no-op implementation.
extern P v150(P controller); // vtable +0x150
extern bool v1c60(P controller);
extern P class_1428ee970();
extern P class_1428ea7f0();
extern P game_state_class();
extern P player_state_class();
extern bool is_a_raw(P object, P target_class); // +0x10 class, ancestry +0x30/+0x38 comparison
extern P current_playlist(P game_state);
extern U64 fun_141680740(P);
extern void fun_141936680(P game_mode, float* location, void* rotation, std::uint32_t* out, P self, U64 captured, U64 zero);
extern void fun_141a3d510(P player_state);
extern void fun_141a5b110(P player_state, const VecBits* location, const VecBits* rotation, std::uint32_t out);
extern bool fun_14199e430(P game_state);
extern bool fun_141b4d240(); // implicit/unknown arguments in decompilation
extern void fun_141a59620(U64 captured, int one);
extern float fun_14070fdf0(P playlist_plus_0x640, int zero);
extern float fun_141996d90(P game_state, P controller);
extern void fun_1419ebd40(P controller);

// Timer delegate lifecycle. The opaque adapters must perform the named native operations,
// including their actual hidden arguments and stack-frame representation.
struct TimerTemps { P clone; int clone_count; P auxiliary_inline; P auxiliary_heap; U64 auxiliary_live; };
extern U64 fun_140f95ae0();
extern P realloc_native(P, std::size_t, unsigned);
extern void free_native(P);
extern void delegate_construct_0x30(P allocation, P controller, U64 callback, std::uint32_t zero, std::uint32_t unknown_stack_11c); // vtables 1459a9e80/1459a9ed8, weak ref 142ee3b30, serial 142bad740
extern void delegate_clone_v40(P allocation, TimerTemps*);
extern void timer_temps_init_weak_null(TimerTemps*); // 142ee3b30; other stack fields zeroed as observed
extern void fun_1446278d0(U64 manager, P handle_0x2eb0, TimerTemps*, float delay, float minus_one, bool zero);
extern void auxiliary_destroy_v10(P);
extern void delegate_destroy_v38(P);
extern std::uint32_t unresolved_stack_11c();
static void schedule_respawn(P controller, float delay) {
  U64 manager = fun_140f95ae0();
  P allocation = realloc_native(nullptr, 0x30, 0);
  if (allocation) delegate_construct_0x30(allocation, controller, 0x1419ebd40ULL, 0, unresolved_stack_11c());
  TimerTemps t;
  t.clone = nullptr;
  t.clone_count = 0;
  if (allocation) delegate_clone_v40(allocation, &t);
  timer_temps_init_weak_null(&t);
  fun_1446278d0(manager, static_cast<char*>(controller)+0x2eb0, &t, delay, -1.0f, false);
  if (t.auxiliary_live) auxiliary_destroy_v10(t.auxiliary_heap ? t.auxiliary_heap : t.auxiliary_inline);
  if (t.clone_count && t.clone) {
    delegate_destroy_v38(t.clone);
    if (t.clone) t.clone = realloc_native(t.clone, 0, 0);
    t.clone_count = 0;
  }
  if (t.clone) free_native(t.clone);
  if (allocation) {
    delegate_destroy_v38(allocation);
    P rest = realloc_native(allocation, 0, 0);
    if (rest) free_native(rest);
  }
}

void SetupClientRespawnTimerAndLocation_1419f5920(P self, float* location, void* rotation, float* reference, char fifth) {
  P world = v150(self), game_mode = nullptr, game_state = nullptr, playlist = nullptr;
  if (world) {
    game_mode = rd<P>(world, 0x128);
    if (game_mode && !is_a_raw(game_mode, class_1428ee970())) game_mode = nullptr;
    game_state = rd<P>(world, 0x130);
    if (!game_state || !is_a_raw(game_state, game_state_class())) {
      game_mode = nullptr; game_state = nullptr;
    } else playlist = current_playlist(game_state);
  }
  if (!game_mode || !playlist || !rd<P>(self, 0x220)) return;
  U64 captured = fun_141680740(self);
  std::uint32_t out = 0;
  fun_141936680(game_mode, location, rotation, &out, self, captured, 0);
  P ps = rd<P>(self, 0x220);
  VecBits loc; std::memcpy(&loc.xy, location, 8);
  wr<std::uint32_t>(rotation, 8, 0); wr<std::uint32_t>(rotation, 0, 0);
  VecBits rot{rd<U64>(rotation, 0), 0};
  std::memcpy(&loc.z, location+2, 4);
  if (ps && is_a_raw(ps, player_state_class())) {
    fun_141a3d510(ps);
    fun_141a5b110(ps, &loc, &rot, out);
  }
  if (!fun_14199e430(game_state)) {
    if (rd<P>(self, 0x3620) && fun_141b4d240()) {
      float dx=location[0]-reference[0], dy=location[1]-reference[1];
      if (std::sqrt(dx*dx+dy*dy) <= rd<float>(self, 0x2d30)) fun_141a59620(captured, 1);
      else fun_1419ebd40(self);
      return;
    }
    if (!rd<std::uint8_t>(playlist, 0x5c9)) {
      float x=location[0], rx=reference[0], y=location[1], ry=reference[1];
      float threshold=rd<float>(self, 0x2d34);
      if (std::fabs(threshold) <= 1e-8f) {
        P w2=v150(self);
        if (w2) {
          P gs2=rd<P>(w2, 0x130);
          if (gs2 && is_a_raw(gs2, game_state_class())) {
            P p2=current_playlist(gs2);
            if (p2) threshold=fun_14070fdf0(static_cast<char*>(p2)+0x640, 0);
          }
        }
      }
      float dx=x-rx, dy=y-ry;
      if (std::sqrt(dx*dx+dy*dy) <= threshold) { fun_141a59620(captured, 1); return; }
    }
    float delay=rd<float>(playlist, 0x5cc);
    if (delay <= 0.0f) fun_1419ebd40(self); else schedule_respawn(self, delay);
    return;
  }
  float elapsed, base;
  if (!fifth) { elapsed=fun_141996d90(game_state,self); base=rd<float>(playlist,0x5cc); }
  else { base=rd<float>(playlist,0x5cc); elapsed=base; }
  float delay=elapsed-base;
  if (delay <= base) delay=base;
  if (delay <= 0.0f) fun_1419ebd40(self); else schedule_respawn(self,delay);
}

extern int log_verbosity();
extern void ready_log(int level, const wchar_t* message);
extern P fun_14237f3b0_first(); // decompiler did not recover argument setup
extern P fun_14237f3b0(P self);
extern U64 fun_142290c00(P);
extern U64 fun_140d91b30(U64);
extern void fun_14226f940(U64);
extern bool fun_14229b1c0();
extern P fun_14447ee40();
extern U64 fun_141e613c0(P);
extern void fun_1421fab00(U64, std::uint32_t, int, U64);
extern void fun_14226f840(U64, int);
extern void fun_1406a9dc0(P, int);
extern bool fun_140d97b10(P multicast, P self);
extern void fun_14167fb70(P multicast, P self, U64 callback, std::uint32_t zero); // hidden temporary argument unresolved

void StartCreativeMatchmakingReadyCheck_1419fb550(P self, U64 arg2, char* result) {
  *result=0;
  if (!v1c60(self)) {
    if (log_verbosity()>2) ready_log(3,L"[AFortPlayerControllerAthena::StartCreativeMatchmakingReadyCheck] Matchmaking Ready check was called by a non-party leader.");
    return;
  }
  P party=fun_14237f3b0_first();
  if (party) {
    U64 service=fun_140d91b30(fun_142290c00(party));
    fun_14226f940(service);
    if (fun_14229b1c0()) {
      if (log_verbosity()>4) ready_log(5,L"[AFortPlayerControllerAthena::StartCreativeMatchmakingReadyCheck] Party was in already readied up, skipping ready check and starting Matchmaking.");
      *result=1;
      P world=v150(self);
      P object=world ? rd<P>(world,0x188) : nullptr;
      if (object && is_a_raw(object,class_1428ea7f0())) {
        P config=fun_14447ee40();
        if (config) fun_1421fab00(fun_141e613c0(object),rd<std::uint32_t>(config,0xb8),1,arg2);
      }
    }
  }
  if (!*result) {
    if (log_verbosity()>4) ready_log(5,L"[AFortPlayerControllerAthena::StartCreativeMatchmakingReadyCheck] Beginning Ready Check for matchmaking.");
    party=fun_14237f3b0(self);
    if (party) fun_14226f840(fun_140d91b30(fun_142290c00(party)),1);
    fun_1406a9dc0(static_cast<char*>(self)+0x2ee0,0);
    P world=v150(self);
    P object=world ? rd<P>(world,0x188) : nullptr;
    if (object && is_a_raw(object,class_1428ea7f0())) {
      P subsystem=reinterpret_cast<P>(fun_141e613c0(object));
      if (subsystem) {
        P event=static_cast<char*>(subsystem)+0x378;
        if (!fun_140d97b10(event,self)) fun_14167fb70(event,self,0x1419e2cc0ULL,0);
      }
    }
  }
}
} // namespace native74
