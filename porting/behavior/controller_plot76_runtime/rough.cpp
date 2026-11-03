// Uncounted behavioral lift. Adapters below are unresolved native contracts, not implementations.
#include <cstdint>
#include <cstring>
#include <atomic>
#include <initializer_list>
namespace controller_plot76 {
struct Controller; struct Object; struct Text; struct Plot; struct Profile; struct DeathReport;
struct String { void* data; int32_t num, max; };
struct Array { void* data; int32_t num, max; };
struct Binding { uint32_t weak[2], name[2]; };
struct Snapshot { alignas(8) unsigned char inline_data[64]; Binding* heap; int32_t num; };
struct RenameContext; struct CallbackTemp; struct SharedText;
extern Profile* creative_profile(Controller*); extern Array* FUN_1421d91b0();
extern void resize_ptr_array(Array&, int32_t); extern void resize_string(String&, int32_t);
extern void free_mem(void*); extern int compare_text(const void*, const void*); extern const void* empty_text();
extern String& plot_string(Plot*, unsigned offset); extern uint64_t plot_state(Plot*);
extern Array* FUN_1419d9d90(Array* out, void* source); extern void* plot_at(Plot*, unsigned offset);
extern void make_text(SharedText&, const wchar_t*, const wchar_t*, const wchar_t*);
extern bool rename_log_enabled(); extern void rename_log_prepare(); extern void rename_log(SharedText&);
extern void FUN_142a52b90(Controller*, int, SharedText&);
struct RefControl { void** vtable; std::atomic<int32_t> strong, weak; };
extern RefControl* text_control(SharedText&); extern void ref_destroy(RefControl*); extern void ref_delete(RefControl*, int);
extern void* make_title_arg(String& temporary, String* title);
extern void FUN_1419b6100(CallbackTemp&, Controller*, void* callback, void* title_arg);
extern void FUN_1419e9980(); extern void FUN_1410eb5f0(RenameContext&);
extern void* callback_allocation(CallbackTemp&); extern int32_t callback_count(CallbackTemp&);
extern void callback_virtual_38(void*); extern void* realloc_zero(void*); extern void callback_set_allocation(CallbackTemp&, void*); extern void callback_reset(CallbackTemp&);
extern void FUN_1429aae00(void*, String*, String*, String*);
extern void* profile_first(Profile*); extern bool context_has_callable(RenameContext&);
extern void* context_inline_callable(RenameContext&); extern void* context_heap_callable(RenameContext&);
extern void callable_virtual_10(void*); extern void* context_allocation(RenameContext&);
extern void FUN_140692d60(RenameContext&);
extern RenameContext& rename_context_storage(); extern CallbackTemp& callback_storage();
extern SharedText& text_storage();
extern void* team_stats_state(Controller*); extern void copy_team_stat_word(void*, const void*);
extern void mark_team_stats_received(void*); extern Array& team_bindings(void*);
extern void snapshot_init_resize(Snapshot&, int32_t); extern bool name_is_none(const uint32_t*);
extern Object* weak_get(const uint32_t*); extern Object* weak_get_even_pending(const uint32_t*, bool);
extern bool weak_valid(const uint32_t*); extern void* find_function(Object*, const uint32_t*);
extern void* find_function_checked(Object*, const uint32_t*); extern void process_event(Object*, void*, void*);
extern void shrink_bindings(Array&);
extern unsigned char* report_destination(Controller*); extern unsigned char* report_bytes(DeathReport*);
extern void FUN_143e8a690(void*, void*); extern void* controller_world(Controller*);
extern void* world_game_state(void*); extern bool is_athena_game_state(void*);
extern void* controller_candidate(Controller*); extern bool candidate_has_class_142a85850(void*);
extern char game_state_virtual_9d8(void*, void*); extern uint8_t game_state_byte_1cb0(void*);
extern void FUN_1406a9dc0(void* delegate, Controller**); extern void* respawn_delegate(Controller*);

static void release_text(SharedText& text) {
  RefControl* c = text_control(text);
  if (c && c->strong.fetch_sub(1) == 1) {
    ref_destroy(c);
    if (c->weak.fetch_sub(1) == 1) ref_delete(c, 1);
  }
}
static void clear_strings(Array& a) {
  auto* strings = static_cast<String*>(a.data);
  for (int32_t i = 0; i < a.num; ++i) if (strings[i].data) free_mem(strings[i].data);
  if (a.data) free_mem(a.data);
  a = {};
}
static void assign_string(String& dst, String& src) {
  if (&dst == &src) return;
  dst.num = src.num;
  if (dst.num != 0 || dst.max != 0) {
    resize_string(dst, dst.num);
    std::memcpy(dst.data, src.data, static_cast<int64_t>(dst.num) * 2);
  }
}
void UpdateCreativePlotName_Implementation(Controller* self, String* title, String* locale, String* island_id) {
  Profile* profile = creative_profile(self);
  if (!profile) return;
  Array* source = FUN_1421d91b0();
  Array plots{}; plots.num = source->num;
  if (plots.num) { resize_ptr_array(plots, plots.num); std::memcpy(plots.data, source->data, int64_t(plots.num) * 8); }
  String s110{}, s130{}, s158{}; Array names{};
  Plot* found = nullptr;
  auto** entries = static_cast<Plot**>(plots.data);
  for (int32_t i = 0; i < plots.num; ++i) {
    Plot* plot = entries[i]; String copy{};
    assign_string(copy, plot_string(plot, 0x90));
    int cmp = compare_text(title->num ? title->data : empty_text(), copy.num ? copy.data : empty_text());
    if (copy.data) free_mem(copy.data);
    if (cmp != 0) continue;
    found = plot;
    assign_string(s110, plot_string(plot, 0x110));
    Array tmp{}; Array* result = FUN_1419d9d90(&tmp, plot_at(plot, 0x120));
    if (&names != result) { clear_strings(names); names = *result; *result = {}; }
    clear_strings(tmp);
    assign_string(s130, plot_string(plot, 0x130));
    assign_string(s158, plot_string(plot, 0x158));
    if (plot_state(plot) != UINT64_C(0x2bca2875f4373fff)) {
      SharedText& text = text_storage();
      make_text(text, L"You cannot rename deleted Islands.", L"Athena", L"IslandRenameDeletedIsland");
      if (rename_log_enabled()) { rename_log_prepare(); rename_log(text); }
      FUN_142a52b90(self, 0, text); release_text(text);
    } else {
      String title_copy{}; void* arg = make_title_arg(title_copy, title);
      CallbackTemp& callback = callback_storage();
      FUN_1419b6100(callback, self, reinterpret_cast<void*>(&FUN_1419e9980), arg);
      RenameContext& context = rename_context_storage(); FUN_1410eb5f0(context);
      void* allocation = callback_allocation(callback);
      if (callback_count(callback) != 0 && allocation) {
        callback_virtual_38(allocation);
        if (callback_allocation(callback)) callback_set_allocation(callback, realloc_zero(callback_allocation(callback)));
        callback_reset(callback);
      }
      if (callback_allocation(callback)) free_mem(callback_allocation(callback));
      FUN_1429aae00(profile_first(profile), title, locale, island_id);
      if (context_has_callable(context)) {
        void* callable = context_heap_callable(context);
        if (!callable) callable = context_inline_callable(context);
        callable_virtual_10(callable);
      }
      if (context_allocation(context)) free_mem(context_allocation(context));
      FUN_140692d60(context);
    }
    break;
  }
  if (!found) {
    SharedText& text = text_storage();
    make_text(text, L"The Island you tried to rename is not one of your valid islands.", L"Athena", L"IslandRenameNotAValidIsland");
    if (rename_log_enabled()) { rename_log_prepare(); rename_log(text); }
    FUN_142a52b90(self, 0, text); release_text(text);
  }
  clear_strings(names);
  if (s158.data) free_mem(s158.data); if (s130.data) free_mem(s130.data); if (s110.data) free_mem(s110.data);
  if (plots.data) free_mem(plots.data);
}
void ClientSendTeamStatsForPlayer_Implementation(Controller* self, const uint64_t* native_arg) {
  void* state = team_stats_state(self); if (!state) return;
  copy_team_stat_word(state, native_arg); mark_team_stats_received(state);
  Array& live = team_bindings(state); int32_t count = live.num;
  if (count > 0) {
    Snapshot snapshot{}; snapshot_init_resize(snapshot, count); snapshot.num = count;
    auto data = [&]() { return snapshot.heap ? snapshot.heap : reinterpret_cast<Binding*>(snapshot.inline_data); };
    std::memcpy(data(), live.data, int64_t(count) * 16);
    for (int32_t i = 0; i >= 0 && i < snapshot.num; ++i) {
      Binding& b = data()[i];
      bool none = name_is_none(b.name) && b.name[1] == 0;
      Object* obj;
      if (!none && (obj = weak_get(b.weak)) && find_function(obj, b.name)) {
        obj = weak_get(b.weak); void* fn = find_function_checked(obj, b.name); process_event(obj, fn, nullptr);
      } else if (none || !weak_get_even_pending(b.weak, true)) {
        auto* bindings = static_cast<Binding*>(live.data);
        for (int32_t j = 0; j < live.num; ++j) {
          Binding& candidate = bindings[j];
          bool same_weak = (candidate.weak[0] == b.weak[0] && candidate.weak[1] == b.weak[1]) ||
                           (!weak_valid(candidate.weak) && !weak_valid(b.weak));
          if (same_weak && candidate.name[0] == b.name[0] && candidate.name[1] == b.name[1]) {
            if (live.num - j - 1 > 0) std::memcpy(&bindings[j], &bindings[live.num - 1], sizeof(Binding));
            --live.num; shrink_bindings(live); break;
          }
        }
      }
    }
    if (snapshot.heap) free_mem(snapshot.heap);
  }
}
static void copy_at(unsigned char* dst, const unsigned char* src, unsigned off, unsigned size) { std::memcpy(dst + off, src + off, size); }
void ClientSetDeathReport_Implementation(Controller* self, DeathReport* report) {
  unsigned char* dst = report_destination(self); unsigned char* src = report_bytes(report);
  for (unsigned off : {0u,4u,8u}) copy_at(dst, src, off, 4);
  copy_at(dst,src,0x10,8); copy_at(dst,src,0x18,8); copy_at(dst,src,0x20,4); copy_at(dst,src,0x24,4);
  copy_at(dst,src,0x28,8); copy_at(dst,src,0x30,8);
  auto* flags = reinterpret_cast<uint32_t*>(dst+0x38); auto source_flags = *reinterpret_cast<uint32_t*>(src+0x38);
  *flags = (*flags & ~1u) | (source_flags & 1u); *flags = (*flags & ~2u) | (source_flags & 2u);
  FUN_143e8a690(dst+0x40,src+0x40);
  copy_at(dst,src,0x60,8); copy_at(dst,src,0x68,4); copy_at(dst,src,0x6c,8); copy_at(dst,src,0x74,4);
  if (!(*reinterpret_cast<float*>(dst) <= 0.0f)) {
    void* gs = world_game_state(controller_world(self)); bool accepted = false;
    if (gs && is_athena_game_state(gs)) {
      void* candidate = controller_candidate(self);
      if (candidate && !candidate_has_class_142a85850(candidate)) candidate = nullptr;
      if (game_state_virtual_9d8(gs,candidate) && game_state_byte_1cb0(gs) != 5) {
        Controller* argument = self; FUN_1406a9dc0(respawn_delegate(self), &argument); accepted = true;
      }
    }
    if (!accepted) *reinterpret_cast<uint32_t*>(dst) = 0;
  }
  void* allocation50; std::memcpy(&allocation50, src+0x50, 8); if (allocation50) free_mem(allocation50);
  void* allocation40; std::memcpy(&allocation40, src+0x40, 8); if (allocation40) free_mem(allocation40);
}
}
