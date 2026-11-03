// Uncounted rough control-flow lifts, NOT buildable integration code.
// Image base 0x140000000; addresses refer to original runtime image.
// Types and helper signatures below are semantic placeholders, not ABI declarations.
#include <cstdint>

struct Volume; struct Actor; struct Pawn; struct World; struct GameMode;
struct Director; struct Invoker; struct GameState; struct WeakActor; struct TimerManager;
struct Name;
namespace unresolved {
  int role(Volume*); World* world(Volume*); GameMode* game_mode(World*);
  bool is_pawn(Actor*), is_building(Actor*), is_athena_pawn(Actor*), is_class_1446acb00(Actor*);
  bool is_athena_game_mode(GameMode*), is_class_1427a1790(Actor*);
  bool actor_has_tag(Actor*, const Name&); Name name(const char*);
  bool has_shared_gameplay_tag(Volume*); // singleton 0x141ed1800 + 0x2050, containers +0x250/+0x260
  void bind_callbacks(Volume*, Actor*, bool); // lift below, 0x141c64450
  void client_membership(Volume*, Actor*, bool); // 0x141c93b20
  int erase_weak_member(Volume*, Actor*); // 0x142ee3b30, 0x1407834a0; count
  void log_missing(Actor*); void broadcast_exit(Volume*, Actor*, void* root, Pawn*);
  void* root_component(Actor*); bool building_stats_enabled(); void decrement_building_category(Volume*, Actor*);
  int& director_count(Volume*); int& navigation_count(Volume*); Director* get_or_create_director(GameMode*);
  void set_director_bit1(Director*); Invoker* nav_invoker(Volume*); void set_invoker_enabled(Invoker*, bool);
  void actor_cleanup_1422c39e0(Actor*, bool); void stat_begin(const wchar_t*); void stat_end();
  bool death_callback_setting(Volume*); // 0x1429da540, vslot +0x610, +0xd0/+0x688; default true
  void add_endplay_binding(Actor*, Volume*); void remove_endplay_binding_if_flagged(Actor*, Volume*);
  void add_building_death_unique(Actor*, Volume*); void remove_building_death(Actor*, Volume*);
  void add_pawn_death_unique(Actor*, Volume*); void remove_pawn_death(Actor*, Volume*);
  void add_pawn_native_callback_if_absent(Actor*, Volume*); void remove_pawn_native_callbacks(Actor*, Volume*);
  void for_each_aux_79(Volume*, void (*)(void*)); void aux_cleanup_1418dd390(void*);
  TimerManager* timer_manager(World*); void clear_timer_if_found_and_zero(TimerManager*, Volume*, int offset);
  // Iteration must preserve the native sparse set and weak resolution; these callbacks do not imply snapshots.
  void for_each_live_weak_member(Volume*, void (*)(Volume*, Actor*));
  GameState* game_state(World*); bool is_athena_game_state(GameState*); void unregister_client_volume(GameState*, Volume*);
  void remove_client_delegate_matching_handle(Volume*); // global delegate 0x142ec9f90, handle +0x400
  void remove_volume_from_two_owner_delegates(Volume*); // owner +0x2a0, delegates +0x478/+0x490
  void cleanup_141c92830(Volume*); void super_endplay_1411ab4c0(Volume*, std::uint32_t);
  int sparse_num(Volume*), sparse_free_num(Volume*);
  void prepare_clear_141c8baa0(Volume*); void compact_or_reserve_141c690a0(Volume*);
  void clear_prepare_141c56b70(Volume*, int live_count); void clear_prepare_140726bb0(Volume*);
  void for_each_occupied_clear_entry(Volume*, void (*)(Volume*, std::uint32_t));
  void clear_entry_141c86ff0(Volume*, std::uint32_t); void reset_sparse_storage_and_hash(Volume*);
}

// 0x141c8abe0: result is removed count on authority, zero on client/null/missing.
int RemoveActor(Volume* self, Actor* actor) {
  using namespace unresolved;
  stat_begin(L"STAT_Volume_RemoveActor");
  int removed = 0;
  if (actor) {
    bind_callbacks(self, actor, false);
    if (role(self) != 3) {
      client_membership(self, actor, false);
    } else {
      removed = erase_weak_member(self, actor);
      if (removed < 1) {
        log_missing(actor); // actual call conditional on verbosity > 2
        removed = 0;
      } else {
        broadcast_exit(self, actor, root_component(actor), is_pawn(actor) ? reinterpret_cast<Pawn*>(actor) : nullptr);
        if (building_stats_enabled() && is_building(actor)) decrement_building_category(self, actor);
        Name navigation = name("AthenaAIRequiresNavigation");
        Name director = name("AthenaAIRequiresAIDirector");
        if (has_shared_gameplay_tag(self)) {
          if (actor_has_tag(actor, director) && --director_count(self) < 1 && has_shared_gameplay_tag(self)) {
            World* w = world(self);
            GameMode* gm = w ? game_mode(w) : nullptr;
            if (gm && is_athena_game_mode(gm)) {
              if (Director* d = get_or_create_director(gm)) set_director_bit1(d);
            }
          }
          if (actor_has_tag(actor, navigation) && --navigation_count(self) < 1) {
            if (Invoker* invoker = nav_invoker(self)) set_invoker_enabled(invoker, false);
          }
          if (is_class_1427a1790(actor)) actor_cleanup_1422c39e0(actor, false);
        }
      }
    }
  }
  stat_end();
  return removed;
}

// 0x141c64450: callback registration/removal. The native body reads actor +0x187
// before its later actor-null check; callers must not assume this accepts null.
void BindMembershipCallbacks(Volume* self, Actor* actor, bool add) {
  using namespace unresolved;
  bool death_enabled = death_callback_setting(self); // evaluated even on removal
  if (add) add_endplay_binding(actor, self); else remove_endplay_binding_if_flagged(actor, self);
  if (is_building(actor)) {
    if (add) add_building_death_unique(actor, self); else remove_building_death(actor, self);
  } else if (is_athena_pawn(actor)) {
    if (death_enabled) {
      if (add) add_pawn_death_unique(actor, self); else remove_pawn_death(actor, self);
    }
    if (add) add_pawn_native_callback_if_absent(actor, self); else remove_pawn_native_callbacks(actor, self);
  }
}

static void UnbindEligible(Volume* self, Actor* actor) {
  if (unresolved::is_class_1446acb00(actor)) unresolved::bind_callbacks(self, actor, false);
}
// 0x141c6f610: EndPlay. Weak iteration resolves each member at use time.
void EndPlay(Volume* self, std::uint32_t reason) {
  using namespace unresolved;
  if (role(self) == 3) {
    for_each_aux_79(self, aux_cleanup_1418dd390); // array +0x3c8
    if (World* w = world(self)) {
      TimerManager* tm = timer_manager(w);
      clear_timer_if_found_and_zero(tm, self, 0x3f8);
      clear_timer_if_found_and_zero(tm, self, 0x3f0);
    }
    for_each_live_weak_member(self, UnbindEligible);
  } else {
    if (World* w = world(self)) {
      if (GameState* gs = game_state(w)) if (is_athena_game_state(gs)) unregister_client_volume(gs, self);
    }
    remove_client_delegate_matching_handle(self);
  }
  remove_volume_from_two_owner_delegates(self); // internally conditional on owner +0x2a0
  cleanup_141c92830(self);
  super_endplay_1411ab4c0(self, reason);
}

// 0x141c86db0: early out on empty; helpers and sparse storage layout unresolved.
void ClearMembership(Volume* self) {
  using namespace unresolved;
  if (sparse_num(self) != sparse_free_num(self)) {
    prepare_clear_141c8baa0(self);
    int live_count = sparse_num(self) - sparse_free_num(self);
    if (live_count > 0) {
      compact_or_reserve_141c690a0(self);
      clear_prepare_141c56b70(self, live_count);
    }
    clear_prepare_140726bb0(self);
    for_each_occupied_clear_entry(self, clear_entry_141c86ff0);
    reset_sparse_storage_and_hash(self);
  }
}
