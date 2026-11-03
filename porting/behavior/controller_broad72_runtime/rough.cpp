// Rough behavioral lift. All extern operations are unresolved binary adapters, not implementations.
#include <cstdint>
#include <cstddef>
#include <initializer_list>

namespace controller_broad72_runtime {
using Obj = void*;
using Word = std::uintptr_t;
struct Pair { Obj object; Obj control; };
struct Array { Obj data; int count; int capacity; };
struct Text { Obj data; int count; int capacity; };
struct Callback { Obj function; std::uint64_t flags; };
struct DelegateLocal { Obj storage; int count; };
struct TimerLocal { Obj opaque[8]; };
extern Obj read_ptr(Obj, std::size_t); extern void write_ptr(Obj, std::size_t, Obj);
extern int read_i32(Obj, std::size_t); extern void write_i32(Obj, std::size_t, int);
extern std::uint8_t read_u8(Obj, std::size_t); extern float read_f32(Obj, std::size_t); extern void write_u64(Obj, std::size_t, std::uint64_t);
extern void call_control_destroy(Obj); extern void call_control_free(Obj, int);
extern void release_memory(Obj); extern Obj realloc_memory(Obj, std::size_t, int);
extern Obj add_bytes(Obj, std::size_t);

static void retain(Obj control) {
  if (control) write_i32(control, 8, read_i32(control, 8) + 1);
}
static void release(Obj control) {
  if (!control) return;
  int strong = read_i32(control, 8) - 1; write_i32(control, 8, strong);
  if (!strong) {
    call_control_destroy(control);
    int weak = read_i32(control, 12) - 1; write_i32(control, 12, weak);
    if (!weak) call_control_free(control, 1);
  }
}
extern void ensure_max_wait_text_tls(); // includes TLS guard, construction, copy, atexit registration
extern Pair get_service_vslot_28(Obj field_5a0);
extern Obj null_path_uninitialized_control(); // decompiler reads uninitialized stack: needs binary resolution
extern bool is_athena_controller(Obj); // class chain check at +10/+30/+38
extern Text copy_max_wait_text(); // copies global count * 2 bytes; zero count leaves null data
extern Obj build_event_payload(Array* second, Array* first, Obj checked_controller, int zero);
extern void service_vslot_c8(Obj service, Text*, Obj payload);
extern Obj array_element_ptr(Obj data, int index, std::size_t stride);
static void free_event_array(Array& a) {
  for (int i = 0; i != a.count; ++i) {
    Obj e = array_element_ptr(a.data, i, 48);
    Obj p = read_ptr(e, 16); if (p) release_memory(p);
    p = read_ptr(e, 0); if (p) release_memory(p);
  }
  if (a.data) release_memory(a.data);
}
void ServerNotifyClientReachedMaxWaitingTimeForDeviceReplication_Implementation(Obj self) {
  ensure_max_wait_text_tls();
  Pair service{};
  Obj initial_control;
  if (!self) initial_control = null_path_uninitialized_control();
  else { service = get_service_vslot_28(read_ptr(self, 0x5a0)); retain(service.control); initial_control = service.control; }
  retain(service.control);
  release(initial_control);
  if (service.object) {
    Obj checked = self && is_athena_controller(self) ? self : nullptr;
    Array first{}, second{};
    Text name = copy_max_wait_text();
    Obj payload = build_event_payload(&second, &first, checked, 0);
    service_vslot_c8(service.object, &name, payload);
    free_event_array(second);
    free_event_array(first);
    // No text destruction is visible in the recovered body.
  }
  release(service.control);
}

extern std::uint32_t global_respawn_default_bits();
extern Obj lookup_config_root(); extern Obj lookup_respawn_config(Obj, Obj field_2ce4);
extern Obj world_vslot_150(Obj); extern Obj timer_manager_fallback(Obj world); // world+320
extern bool timer_lookup(Obj manager, Obj handle_address);
extern bool timer_lookup_third_unrecovered_args();
extern void clear_timer(Obj manager, Word handle);
extern Word read_word(Obj, std::size_t); extern void write_word(Obj, std::size_t, Word);
extern void log_fade_if_verbosity_above_4();
extern void camera_fade_vslot_7c0(Obj camera, std::uint32_t from_bits, std::uint32_t to_bits, float duration);
extern Obj construct_fade_delegate_30(Obj allocation, Obj self, std::uint32_t config_bits);
extern void delegate_clone_vslot_40(Obj delegate, DelegateLocal* out);
extern void init_timer_local(TimerLocal*); // weak-init(null), two zero words, null pointer
extern void set_timer(Obj manager, Obj handle_address, DelegateLocal*, float duration, TimerLocal*);
extern void destroy_timer_local_if_present(TimerLocal*); // inline/heap vslot +10 selection
extern void delegate_destroy_vslot_38(Obj);
extern void final_respawn_call(Obj self, Word arg, int one);
extern void check_stack_cookie();
void ClientHideScreenWhileRespawning_Implementation(Obj self, Word arg) {
  std::uint32_t bits = global_respawn_default_bits(); float duration = 1.0f;
  Obj config = lookup_respawn_config(lookup_config_root(), read_ptr(self, 0x2ce4));
  if (config) { bits = static_cast<std::uint32_t>(read_i32(config, 0x14)); duration = read_f32(config, 0x18); }
  Obj world = world_vslot_150(self);
  if (world) {
    Obj owner = read_ptr(world, 0x188);
    Obj manager = owner ? read_ptr(owner, 0xc0) : timer_manager_fallback(world);
    for (std::size_t off : {std::size_t(0x2e98), std::size_t(0x2ea0)}) {
      if (timer_lookup(manager, add_bytes(self, off))) clear_timer(manager, read_word(self, off));
      write_word(self, off, 0);
    }
    if (timer_lookup_third_unrecovered_args()) clear_timer(manager, read_word(self, 0x2ea8));
    write_word(self, 0x2ea8, 0);
    log_fade_if_verbosity_above_4();
    Obj camera = read_ptr(self, 0x2b0);
    camera_fade_vslot_7c0(camera, static_cast<std::uint32_t>(read_i32(camera, 0x254)), 0x3f800000, duration);
    Obj allocation = realloc_memory(nullptr, 0x30, 0);
    Obj delegate = allocation ? construct_fade_delegate_30(allocation, self, bits) : nullptr;
    DelegateLocal local{nullptr, 0};
    if (delegate) delegate_clone_vslot_40(delegate, &local);
    TimerLocal timer; init_timer_local(&timer);
    set_timer(manager, add_bytes(self, 0x2e88), &local, duration + 0.001f, &timer);
    destroy_timer_local_if_present(&timer);
    Obj active = local.count ? local.storage : nullptr;
    if (active || (local.count && local.storage)) {
      delegate_destroy_vslot_38(active ? active : local.storage);
      if (local.storage) local.storage = realloc_memory(local.storage, 0, 0);
      local.count = 0;
      if (local.storage) release_memory(local.storage);
    } else if (!local.count && local.storage) release_memory(local.storage);
    if (delegate) {
      delegate_destroy_vslot_38(delegate);
      Obj residue = realloc_memory(delegate, 0, 0); if (residue) release_memory(residue);
    }
  }
  final_respawn_call(self, arg, 1);
  check_stack_cookie();
}

extern void p_finish(Obj stack);
extern Obj session_data(Obj self); extern Obj linked_volume(Obj self); extern Obj changed_delegate(Obj self);
extern Obj weak_get(Obj weak); extern void clear_old_plot(Obj self, Obj session);
extern bool controller_vslot_6a8(Obj self); extern Obj controller_field_gap_288(Obj self);
extern Obj plot_class(); extern bool class_chain_matches(Obj object, Obj type);
extern void update_plot_at_448(Obj field, Obj world, Obj volume);
extern Obj find_plot_context(Obj self); extern Obj minigame_from_volume(Obj volume);
extern Obj minigame_from_new_volume_unrecovered(); // call has no recovered arguments
extern const wchar_t* name_separator(); // DAT_1459a98c4
extern const wchar_t* find_wide(const wchar_t*, const wchar_t*);
extern Obj make_name(const wchar_t* substring, int one);
extern void remove_dynamic(Obj delegate, Obj self, Obj name);
extern Obj started_callback(); extern Obj ended_callback();
extern std::uint64_t unrecovered_callback_flags_high();
extern void add_dynamic(Obj delegate, Obj self, Callback*, Obj name);
extern void invoke_started(Obj self); extern void plot_changed_refresh(Obj self);
extern void broadcast_volume(Obj delegate, Obj* volume);
extern void assign_weak(Obj weak, Obj volume);
static Obj callback_name(const wchar_t* qualified) {
  const wchar_t* match = find_wide(qualified, name_separator());
  const wchar_t* last = nullptr;
  while (match) { last = match; match = find_wide(reinterpret_cast<const wchar_t*>(reinterpret_cast<Word>(match) + 2), name_separator()); }
  // The machine code adds four bytes even if no separator is found.
  auto address = reinterpret_cast<Word>(last) + 4;
  return make_name(reinterpret_cast<const wchar_t*>(address), 1);
}
static void reset_session(Obj session) {
  write_u64(session, 0, 0xffffffffULL); write_u64(session, 8, 0); write_i32(session, 0x10, 0);
}
void OnRep_CreativePlotLinkedVolume(Obj self, Obj stack, Obj /*result*/) {
  p_finish(stack);
  Obj session = session_data(self);
  Obj old_volume = weak_get(session);
  Obj current = linked_volume(self);
  if (old_volume != current) {
    if (weak_get(session)) clear_old_plot(self, session);
    reset_session(session);
    if (controller_vslot_6a8(self)) {
      Obj field = controller_field_gap_288(self);
      if (field && class_chain_matches(field, plot_class()))
        update_plot_at_448(add_bytes(field, 0x448), world_vslot_150(self), linked_volume(self));
    }
    if (find_plot_context(self)) {
      Obj game = old_volume ? minigame_from_volume(old_volume) : nullptr;
      if (game) {
        remove_dynamic(add_bytes(game, 0x378), self, callback_name(L"&AFortPlayerControllerAthena::OnMinigameStarted"));
        remove_dynamic(add_bytes(game, 0x388), self, callback_name(L"&AFortPlayerControllerAthena::OnMinigameEnded"));
      }
      game = linked_volume(self) ? minigame_from_new_volume_unrecovered() : nullptr;
      if (game) {
        Callback cb{started_callback(), unrecovered_callback_flags_high() & 0xffffffff00000000ULL};
        add_dynamic(add_bytes(game, 0x378), self, &cb, callback_name(L"&AFortPlayerControllerAthena::OnMinigameStarted"));
        cb = {ended_callback(), cb.flags & 0xffffffff00000000ULL};
        add_dynamic(add_bytes(game, 0x388), self, &cb, callback_name(L"&AFortPlayerControllerAthena::OnMinigameEnded"));
        if (read_u8(game, 0x870)) invoke_started(self);
      }
    }
    plot_changed_refresh(self);
    current = linked_volume(self);
  }
  broadcast_volume(changed_delegate(self), &current);
  current = linked_volume(self);
  reset_session(session);
  assign_weak(session, current);
}
} // namespace controller_broad72_runtime
