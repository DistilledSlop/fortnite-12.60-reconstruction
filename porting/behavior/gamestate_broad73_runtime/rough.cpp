// Behavioral draft of five exec bodies, not a native ABI implementation.
#include <cstdint>
#include <cstddef>
namespace broad73 {
using byte = unsigned char;
using Object = void*;
struct Context; struct Frame;
struct RefController; // native shared reference controller, layout and atomic ABI unresolved
// Opaque storage is adapter-owned: the decompiler's local sizes/parameter annotations
// are not reliable enough to assert a C++ layout or types for these aggregates.
struct MessageLocal; struct MessageCopy; struct TextLocal; struct TagLocal; struct StateLocal;
struct Iterator;
extern MessageLocal* message_local(); extern MessageCopy* message_copy_storage();
extern TextLocal* text_local(); extern TagLocal* tag_local(); extern StateLocal* state_local();
extern void initialize_message_first_byte_zero(MessageLocal*);
extern void initialize_message_text_subobject(MessageLocal*); // FUN_142c0c240 at +8
extern void initialize_message_remaining_observed_fields(MessageLocal*); // zeros and float bits 0x40a00000
extern void step_message(Frame*, MessageLocal*);
extern void step_object_zeroed(Frame*, Object*);
extern void finish(Frame*); // P_FINISH
extern void copy_message_first_byte_and_first_word(MessageCopy*, MessageLocal*);
extern RefController* message_ref_controller(MessageLocal*);
extern void copy_message_ref_controller(MessageCopy*, RefController*);
extern void copy_message_remaining_observed_fields(MessageCopy*, MessageLocal*);
extern void retain_strong_atomic(RefController*);
extern int32_t decrement_strong_atomic_old(RefController*);
extern int32_t decrement_weak_atomic_old(RefController*);
extern void destroy_shared_object_v0(RefController*);
extern void destroy_controller_v8(RefController*, int); // argument 1
extern void native_context_tutorial_message(Context*, MessageCopy*, Object); // FUN_14199a400, actual body unresolved
extern void native_game_mode_message(Context*, MessageCopy*); // FUN_14199a760, actual body unresolved
extern void state_zero(StateLocal*); extern void clear_recent_address(Frame*);
extern void step_state(Frame*, StateLocal*); extern void* recent_address(Frame*);
extern void initialize_text(TextLocal*); // FUN_142c0c240
extern void step_text(Frame*, TextLocal*); extern void* read_pointer_at(void*);
extern void initialize_tag_first_byte_zero(TagLocal*); extern void step_tag(Frame*, TagLocal*);
extern byte tag_first_byte(TagLocal*);
extern void native_set_location_text(Context*, void*, void*, byte); // FUN_1419aeb90, actual body/ABI unresolved
extern RefController* text_ref_controller(TextLocal*);
static void release(RefController* c) {
  if (c && decrement_strong_atomic_old(c) == 1) {
    destroy_shared_object_v0(c);
    if (decrement_weak_atomic_old(c) == 1) destroy_controller_v8(c, 1);
  }
}
static void prepare(MessageLocal* m) {
  initialize_message_first_byte_zero(m);
  initialize_message_text_subobject(m);
  initialize_message_remaining_observed_fields(m);
}
static void copy_after_finish(MessageCopy* dst, MessageLocal* src) {
  RefController* c = message_ref_controller(src);
  copy_message_ref_controller(dst, c);
  if (c) retain_strong_atomic(c);
  copy_message_remaining_observed_fields(dst, src);
}
void HandleContextTutorialGameModeMessageRequest(Context* ctx, Frame* stack) {
  MessageLocal* m = message_local(); MessageCopy* copy = message_copy_storage();
  prepare(m);
  step_message(stack, m); // decompiler labels this aggregate OwnedController
  Object argument = nullptr; step_object_zeroed(stack, &argument); // labelled MessageData
  copy_message_first_byte_and_first_word(copy, m);
  finish(stack);
  copy_after_finish(copy, m);
  native_context_tutorial_message(ctx, copy, argument);
  release(message_ref_controller(m));
}
void HandleGameModeMessageRequest(Context* ctx, Frame* stack) {
  MessageLocal* m = message_local(); MessageCopy* copy = message_copy_storage();
  prepare(m); step_message(stack, m);
  copy_message_first_byte_and_first_word(copy, m);
  finish(stack); copy_after_finish(copy, m);
  native_game_mode_message(ctx, copy);
  release(message_ref_controller(m));
}
void SetLocationText(Context* ctx, Frame* stack) {
  StateLocal* state = state_local(); TextLocal* text = text_local(); TagLocal* tag = tag_local();
  state_zero(state); clear_recent_address(stack); step_state(stack, state);
  void* state_arg = recent_address(stack); if (!state_arg) state_arg = state;
  initialize_text(text); clear_recent_address(stack); step_text(stack, text);
  initialize_tag_first_byte_zero(tag);
  // Native dereferences MostRecentPropertyAddress without a null check here.
  void* text_arg = read_pointer_at(recent_address(stack)); if (!text_arg) text_arg = text;
  step_tag(stack, tag); finish(stack);
  native_set_location_text(ctx, state_arg, text_arg, tag_first_byte(tag));
  release(text_ref_controller(text));
}
// All field names/offset bases are the decompiler's provisional labels.
extern byte* named_poi_delegate(Context*); extern void broadcast_poi(byte*, int); // FUN_1406a9dc0
extern Object virtual_af0(Context*); extern Object virtual_150(Context*);
extern Iterator* iterator_storage(); extern void make_iterator(Object, Iterator*); // FUN_14468fb70
extern int32_t iterator_index(Iterator*); extern int32_t iterator_count(Iterator*);
extern void* iterator_weak_entry(Iterator*, int32_t); extern void set_iterator_index(Iterator*, int32_t);
extern Object weak_get(void*); extern bool is_target_class(Object); // FUN_142a3d9f0 ancestry check
extern byte* golden_tags(Context*); extern Object poi_lookup(Object, void*, Object); // FUN_141ed6bb0
extern void poi_set(Object, int); // FUN_141ed0f40
extern byte* spectator_delegate_base(Context*); extern uint64_t spectator_info_word(Context*);
extern bool callback_v50(Object, uint64_t); extern void compact_callbacks(void*, int); // FUN_140685370
extern int32_t read_i32(void*); extern void write_i32(void*, int32_t);
extern void* read_ptr(void*);
void OnRep_GoldenPoiLocationTags(Context* ctx, Frame* stack) {
  finish(stack);
  byte* delegate = named_poi_delegate(ctx);
  if (read_i32(delegate + 8) > 0) broadcast_poi(delegate, 0);
  Object manager = virtual_af0(ctx);
  if (manager) {
    Object world = virtual_150(ctx); Iterator* it = iterator_storage(); make_iterator(world, it);
    while (iterator_index(it) >= 0 && iterator_index(it) < iterator_count(it)) {
      Object obj = weak_get(iterator_weak_entry(it, iterator_index(it)));
      if (obj && !is_target_class(obj)) obj = nullptr;
      byte* tags = golden_tags(ctx); int32_t count = read_i32(tags + 8);
      for (int32_t i = 0; i != count; ++i) {
        Object found = poi_lookup(manager, static_cast<byte*>(read_ptr(tags)) + int64_t(i) * 8, obj);
        if (found) poi_set(found, 1);
      }
      set_iterator_index(it, iterator_index(it) + 1);
    }
  }
}
void OnRep_BroadcastSpectatorInfo(Context* ctx, Frame* stack) {
  finish(stack); bool needs_compaction = false;
  uint64_t info = spectator_info_word(ctx); byte* base = spectator_delegate_base(ctx);
  write_i32(base + 0x3c, read_i32(base + 0x3c) + 1);
  int32_t depth = read_i32(base + 0x3c);
  int64_t i = int64_t(read_i32(base + 0x30) - 1);
  if (i >= 0) {
    for (; i >= 0; --i) {
      byte* array = static_cast<byte*>(read_ptr(base + 0x28)); byte* entry = array + i * 0x10;
      if (read_i32(entry + 8) == 0 || !read_ptr(entry)) needs_compaction = true;
      else if (!callback_v50(read_ptr(entry), info)) needs_compaction = true;
    }
    depth = read_i32(base + 0x3c);
  }
  write_i32(base + 0x3c, depth - 1);
  if (needs_compaction) compact_callbacks(base + 0x28, 0);
}
}
