// Uncounted behavioral drafts; adapters and field addresses are not ABI declarations.
#include <cstdint>
#include <cstddef>
#include <cstring>
namespace controller_misc77 {
using P = void*; using U64 = std::uint64_t; using B = std::uint8_t;
template<class T> T rd(P p, std::size_t o) { T v; std::memcpy(&v, static_cast<char*>(p)+o, sizeof v); return v; }
template<class T> void wr(P p, std::size_t o, T v) { std::memcpy(static_cast<char*>(p)+o, &v, sizeof v); }
extern P add(P, std::size_t);
extern P previous_pawn_field(P); extern P win_screen_delay_field(P); extern P battle_lab_input_field(P);
extern P handshake_field(P); extern P skydive_leader_field(P); extern P my_fort_pawn_field(P);
extern P v150(P); extern P class_1428ee970(); extern P class_142a85850();
extern P class_142a8ae70(); extern P athena_controller_class();
extern bool is_a_raw(P object, P target); // native class depth and ancestry slot comparison; no null fallback
extern P fun_1419d3ff0(P); extern P fun_1419d3ff0_noargs(); extern P fun_1419967f0(P); extern float fun_1419922b0(P, int);
extern U64 fun_140f95ae0(); extern U64 fun_140f95ae0_self(P); // native prototype/implicit self unresolved
extern P fun_144621fb0(U64, P); extern void fun_140684de0(P);
extern void fun_1419f81e0(); // no explicit arguments recovered
extern P mem_realloc(P, std::size_t, unsigned); extern void mem_free(P);
extern void weak_init_142ee3b30(P destination, P object); extern U64 fun_142bad740();
extern U64 unresolved_delegate_tail_teamlost(); extern U64 unresolved_delegate_tail_handshake();
extern void write_vtable(P, U64); extern void write_callback_pair(P, U64 callback, std::uint32_t zero, U64 unknown_tail);
extern void delegate_clone_v40(P, P* clone, int* count); extern void delegate_destroy_v38(P);
extern void auxiliary_destroy_v10(P); // native vcall uses implicit arguments; unresolved
struct TimerTemps { P clone; int count; alignas(8) unsigned char weak[8]; U64 zero; U64 live; P heap; alignas(8) unsigned char inline_object[32]; };
extern void fun_1446278d0(U64 manager, P handle, TimerTemps*, float delay, float parameter, B flag); // hidden stack arguments/layout unresolved
static void finish_timer(P allocation, TimerTemps& t) {
  if (t.live) auxiliary_destroy_v10(t.heap ? t.heap : static_cast<P>(t.inline_object));
  P selected = t.count ? t.clone : nullptr;
  if (selected) {
    delegate_destroy_v38(selected);
    if (t.clone) t.clone = mem_realloc(t.clone, 0, 0);
    t.count = 0;
  }
  // If count != 0 and clone == null, the native branch skips the free block.
  if ((!t.count || selected) && t.clone) mem_free(t.clone);
  if (allocation) { delegate_destroy_v38(allocation); P rest = mem_realloc(allocation, 0, 0); if (rest) mem_free(rest); }
}
static P make_delegate(P self, std::size_t size, U64 base, U64 derived, U64 callback, U64 (*tail)(), bool extended) {
  P p = mem_realloc(nullptr, size, 0);
  if (p) {
    write_vtable(p, base); weak_init_142ee3b30(add(p, 8), self);
    write_callback_pair(add(p, 0x10), callback, 0, tail());
    if (extended) { wr<U64>(p, 0x20, 0); wr<U64>(p, 0x28, 0); wr<std::uint16_t>(p, 0x30, 0x131); }
    wr<U64>(p, extended ? 0x38 : 0x28, fun_142bad740()); write_vtable(p, derived);
  }
  return p;
}
static void schedule(U64 manager, P handle, P allocation, float delay, float parameter, B flag) {
  TimerTemps t; t.clone = nullptr; t.count = 0;
  if (allocation) delegate_clone_v40(allocation, &t.clone, &t.count);
  weak_init_142ee3b30(t.weak, nullptr); t.zero = 0; t.live = 0; t.heap = nullptr;
  fun_1446278d0(manager, handle, &t, delay, parameter, flag);
  finish_timer(allocation, t);
}
void ClientNotifyTeamLost_1419c5860(P self) {
  if (!rd<B>(win_screen_delay_field(self), 4)) {
    P handle = add(previous_pawn_field(self), 0x28);
    U64 local = rd<U64>(handle, 0);
    U64 manager = fun_140f95ae0();
    P lookup = fun_144621fb0(manager, &local);
    if ((!lookup || rd<B>(lookup, 1) == 2)) {
      P state = fun_1419d3ff0(self);
      if (state) { P sub = fun_1419967f0(state);
        if (sub && rd<P>(sub, 0x48)) {
          float delay = fun_1419922b0(state, 0); if (delay <= 0.01f) delay = 0.01f;
          P delegate = make_delegate(self, 0x40, 0x145e28ce0ULL, 0x145e28d38ULL, 0x142a53870ULL, unresolved_delegate_tail_teamlost, true);
          U64 timer_manager = fun_140f95ae0_self(self);
          schedule(timer_manager, handle, delegate, delay, -1.0f, 0);
        }
      }
    }
  }
  fun_140684de0(add(battle_lab_input_field(self), 0x58));
}
void ClientRequestLevelStreamingHandshake_1419c74a0(P self) {
  if (rd<B>(self, 0xf0) == 3) return;
  fun_1419f81e0();
  P world = v150(self); P x = rd<P>(world, 0x188);
  U64 manager = x ? rd<U64>(x, 0xc0) : rd<U64>(world, 800);
  P delegate = make_delegate(self, 0x30, 0x1459a9e80ULL, 0x1459a9ed8ULL, 0x1419c4540ULL, unresolved_delegate_tail_handshake, false);
  schedule(manager, add(handshake_field(self), 8), delegate, 0.1f, 2.0f, 1);
}
extern P fun_1419982c0(P, U64); // second argument upper bits unknown on failed cast
extern U64 unresolved_rdx_upper_clear(); extern P weak_get(P); extern void array_grow(P* data, int* count, int* capacity, int old_count);
extern void fun_1419e89a0(P);
void ServerClearSkydiveLeader_1419f1170(P self) {
  P leader = rd<P>(skydive_leader_field(self), 0); if (!leader) return;
  wr<B>(leader, 8, 1);
  int active = 0; P state = fun_1419d3ff0_noargs();
  if (state) {
    P ps = rd<P>(self, 0x220); U64 selector;
    if (ps) {
      P target = class_142a8ae70(); P ps_class = rd<P>(ps, 0x10);
      selector = is_a_raw(ps, target) ? U64(rd<B>(ps, 0xfdc))
                                    : reinterpret_cast<U64>(ps_class) & ~U64(0xff);
    } else selector = unresolved_rdx_upper_clear() & ~U64(0xff);
    P source = fun_1419982c0(state, selector);
    P weak_data = rd<P>(source, 0); int weak_count = rd<int>(source, 8);
    P* data = nullptr; int count = 0, capacity = 0;
    for (int i = 0; i != weak_count; ++i) {
      P object = weak_get(add(weak_data, std::size_t(i)*8));
      if (object) { int next = count + 1; if (capacity < next) array_grow(reinterpret_cast<P*>(&data), &count, &capacity, count); data[count] = object; count = next; }
    }
    for (int i = 0; i != count; ++i) {
      P controller = rd<P>(data[i], 0xe0);
      if (controller && is_a_raw(controller, athena_controller_class()) && !rd<B>(controller, 0x2be8)) ++active;
    }
    if (data) mem_free(data);
  }
  if (active <= 1) {
    leader = rd<P>(skydive_leader_field(self), 0); P controller = rd<P>(leader, 0xe0);
    if (controller && is_a_raw(controller, athena_controller_class())) { wr<U64>(controller, 0x2be0, 0); fun_1419e89a0(controller); }
  }
  wr<P>(skydive_leader_field(self), 0, nullptr); fun_1419e89a0(self);
}
extern P fun_141991c10(P, U64);
extern B v9e0(P, P); extern B vb28(P, P, B); extern P fun_141ed1800();
struct PairArrays { P first; U64 first_meta; P second; U64 second_meta; };
struct EffectValue { U64 words[4]; };
extern void fun_143e8b3b0(PairArrays*, P); extern void fun_143e8a690(EffectValue*, PairArrays*);
extern void pawn_va78(P pawn, EffectValue*, P controller, int zero);
void ServerSetTeam_1419f2540(P self, B team) {
  if (!rd<P>(my_fort_pawn_field(self), 0)) return;
  P world = v150(self); if (!world) return;
  P mode = rd<P>(world, 0x128); if (!mode || !is_a_raw(mode, class_1428ee970())) return;
  P limits = fun_141991c10(self, ((reinterpret_cast<U64>(rd<P>(mode, 0x10)) & ~U64(0xff)) | 1));
  bool in_range = team >= rd<B>(limits, 200) && team <= rd<B>(limits, 0xc9);
  B allowed = 0; P ps = rd<P>(self, 0x220);
  if (ps && is_a_raw(ps, class_142a85850())) allowed = v9e0(mode, ps);
  if (in_range && allowed && vb28(mode, self, team) && !(rd<B>(rd<P>(my_fort_pawn_field(self), 0), 0x520) & 8)) {
    PairArrays arrays{nullptr, 0, nullptr, 0};
    P config = fun_141ed1800(); fun_143e8b3b0(&arrays, add(config, 0x380));
    config = fun_141ed1800(); fun_143e8b3b0(&arrays, add(config, 0x410));
    EffectValue value{{0,0,0,0}}; fun_143e8a690(&value, &arrays);
    pawn_va78(rd<P>(my_fort_pawn_field(self), 0), &value, self, 0);
    if (arrays.second) mem_free(arrays.second); if (arrays.first) mem_free(arrays.first);
  }
}
}
