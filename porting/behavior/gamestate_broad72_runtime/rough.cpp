// Rough behavioral lift of three exec bodies. Adapters are unresolved ABI contracts,
// not implementations. Offsets below are relative to the identified field/object.
#include <cstdint>
#include <cstddef>
#include <cstring>
namespace broad72 {
using byte = unsigned char;
using Object = void*;
struct Array { byte* data; int32_t num; int32_t max; };
using Text = void*; // Adapter-owned handle to native ref-counted text, not its ABI layout.
struct Context;
extern void p_finish(Context*);
extern byte* cosmetic_report(Context*); // CosmeticUsageReport field
extern byte* callback_storage(Context*); // decompiler labels SmoothedWorldTimeSecondsDrift
extern Object map_info(Context*);
extern byte* loot_info(Context*);
extern Object winning_player_state(Context*);
extern byte* winner_announced(Context*);
extern byte* update_named_poi_color_delegate(Context*);
extern void* alloc_string(Array&, int32_t); // FUN_140690ac0, exact capacity semantics unresolved
extern void free_memory(void*);
extern void report_prepare(byte*); // FUN_1419aaef0
extern uint64_t item_key(Object); // FUN_142ee3b30
extern int32_t report_find(byte* map, uint64_t key); // FUN_14172cd90
extern Array make_string(const char16_t*, int32_t); // native capacity/terminator behavior
extern Array format_header(const char16_t*, const char16_t*, int32_t);
extern Array format_no_variants();
extern Array format_variant(const char16_t*, const char16_t*, const char16_t*, int32_t);
extern Array name_string(const void*); // FUN_142ce9f90, owning string
extern const char16_t* empty_string(); // DAT_1459a4b30
extern void append(Array&, const char16_t*, int32_t); // FUN_142ba9300
extern Object find_variant(Object item, uint64_t channel); // FUN_141d507d0
extern Text unknown_text(); // string -> text via FUN_142c26df0
extern Text variant_channel_text(Object variant); // vtable +0x328
extern Text variant_value_text(Object variant, uint64_t value); // vtable +0x330
extern Text format_variant_text(Text& channel, Text& value); // {0}({1})
extern const Array& text_string(Text&); // FUN_142c40540
extern void release_text(Text&); // atomic strong/weak release; destructor then vtable +8
extern void callback_compact(byte* storage, int32_t); // FUN_140685370
extern bool callback_invoke(Object callback, byte* report); // vtable +0x50
extern void resize_loot_array(Array&, int32_t); // FUN_141471050
extern Array winner_name(Object); // FUN_1444ac7a0, owned result
// Models the null branch's local selection, move/extraction, and conditional cleanup.
// The decompilation reads an apparently uninitialized local; its result is unresolved.
extern Array winner_null_branch_unresolved();
extern bool is_winner_type(Object); // FUN_142a8ae70 class ancestry test
extern Object context_v150(Context*);
extern Object lookup_world(Object); // FUN_14463f750(global, v150)
extern Object lookup_controller(Object, Object); // FUN_14447f220
extern bool is_athena_controller(Object); // StaticClass ancestry test
extern Array controller_winner_name(Object, Object); // FUN_1423d5330, owned result
extern void broadcast_winner(byte*, Array&); // FUN_1406a9dc0
extern void winner_fallback(Context*); // FUN_142919d10

template<class T> static T& at(void* p, size_t off) { return *reinterpret_cast<T*>(static_cast<byte*>(p) + off); }
static void destroy(Array& a) { if (a.data) free_memory(a.data); a.data = nullptr; }
static const char16_t* chars(const Array& a) { return a.num ? reinterpret_cast<const char16_t*>(a.data) : empty_string(); }
static void append_owned(Array& dst, Array& src) { append(dst, reinterpret_cast<const char16_t*>(src.data), src.num ? src.num - 1 : 0); destroy(src); }

void OnRep_CosmeticUsage(Context* ctx) {
  p_finish(ctx);
  byte* report = cosmetic_report(ctx);
  report[0x80] = 1;
  report_prepare(report);
  Array output = make_string(u"-- BEGIN COSMETIC USAGE REPORT --\n", 0x23);
  auto items = at<Object*>(report, 0);
  const int32_t item_count = at<int32_t>(report, 8);
  const int32_t priority_count = at<int32_t>(report, 0x78);
  int32_t ordinal = 1;
  for (int32_t i = 0; i != item_count; ++i) {
    Object item = items[i];
    if (!item) continue;
    uint64_t key = item_key(item);
    int32_t index = report_find(report + 0x10, key);
    // Native output dereferences null on a missing key; preserve that contract.
    byte* record = index == -1 ? nullptr : at<byte*>(report, 0x10) + int64_t(index) * 0x28;
    Array item_name = name_string(static_cast<byte*>(item) + 0x18);
    Array header = format_header(ordinal <= priority_count ? u"PRIORITY " : u"", chars(item_name), at<int32_t>(record, 0x18));
    append_owned(output, header);
    destroy(item_name);
    int32_t count = at<int32_t>(record, 0x10);
    if (!count) {
      Array line = format_no_variants();
      append_owned(output, line);
    } else {
      byte* first = at<byte*>(record, 8);
      byte* end = first + int64_t(count) * 0x14;
      for (byte* entry = first; entry != end; entry += 0x14) {
        Text description = unknown_text();
        Object variant = find_variant(item, at<uint64_t>(entry, 0));
        if (variant) {
          // vtable value query occurs before channel query. Formatting owns temporary
          // text/refcount copies and releases them in the native order (see handoff).
          Text value = variant_value_text(variant, at<uint64_t>(entry, 8));
          Text channel = variant_channel_text(variant);
          Text formatted = format_variant_text(channel, value);
          release_text(channel);
          release_text(value);
          release_text(description);
          description = static_cast<Text&&>(formatted);
        }
        const Array& description_string = text_string(description);
        Array value_name = name_string(entry + 8);
        Array channel_name = name_string(entry);
        Array line = format_variant(chars(channel_name), chars(value_name), chars(description_string), at<int32_t>(entry, 0x10));
        append_owned(output, line);
        destroy(channel_name);
        destroy(value_name);
        release_text(description);
      }
    }
    ++ordinal;
  }
  append(output, u"-- END COSMETIC USAGE REPORT --", 0x1f);
  byte* storage = callback_storage(ctx);
  ++at<int32_t>(storage, 0x18);
  bool compact = false;
  for (int64_t i = int64_t(at<int32_t>(storage, 0xc)) - 1; i >= 0; --i) {
    byte* entries = at<byte*>(storage, 4);
    byte* entry = entries + i * 0x10;
    Object callback = at<Object>(entry, 0);
    if (!at<int32_t>(entry, 8) || !callback || !callback_invoke(callback, report)) compact = true;
  }
  --at<int32_t>(storage, 0x18);
  if (compact) callback_compact(storage + 4, 0);
  destroy(output);
}

static void copy_loot(Array& dst, Array& src) {
  if (&dst == &src) return;
  int32_t count = src.num;
  byte* data = src.data;
  int32_t old_max = dst.max;
  dst.num = count;
  if (!count && !old_max) dst.max = 0;
  else {
    resize_loot_array(dst, count);
    for (int32_t i = 0; i != count; ++i)
      std::memcpy(dst.data + int64_t(i) * 0x88, data + int64_t(i) * 0x88, 0x88);
  }
}
void OnRep_MapInfo(Context* ctx) {
  p_finish(ctx);
  Object map = map_info(ctx);
  if (!map) return;
  byte* loot = loot_info(ctx);
  copy_loot(at<Array>(loot, 0), at<Array>(map, 0x298));
  // Native reloads MapInfo before the second array.
  copy_loot(at<Array>(loot, 0x10), at<Array>(map_info(ctx), 0x2a8));
}

static Array clone_utf16(const Array& src) {
  Array result{nullptr, src.num, 0};
  if (src.num) { alloc_string(result, src.num); std::memcpy(result.data, src.data, int64_t(src.num) * 2); }
  return result;
}
void OnRep_WinningPlayerState(Context* ctx) {
  p_finish(ctx);
  Object winner = winning_player_state(ctx);
  Array name = winner ? winner_name(winner) : winner_null_branch_unresolved();
  winner = winning_player_state(ctx);
  if (winner && is_winner_type(winner)) {
    Object world_arg = context_v150(ctx);
    Object world = lookup_world(world_arg);
    if (world) {
      Object controller = lookup_controller(world, world_arg);
      if (controller && is_athena_controller(controller)) {
        Array replacement = controller_winner_name(controller, winner);
        destroy(name);
        name = replacement;
      }
    }
  }
  Array intermediate = clone_utf16(name);
  Array broadcast = clone_utf16(intermediate);
  broadcast_winner(winner_announced(ctx), broadcast);
  destroy(broadcast);
  destroy(intermediate);
  winner = winning_player_state(ctx);
  if ((!winner || !at<Object>(winner, 0xe0)) && !update_named_poi_color_delegate(ctx)[0x10]) winner_fallback(ctx);
  destroy(name);
}
} // namespace broad72
