// Connected behavioral lift; native ABI and stack layout remain unresolved.
#include <cstddef>
#include <cstdint>
#include <cmath>

namespace gamestate_native74 {
using Byte = std::uint8_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;
using Ptr = void*;

// The adapter must read live native memory at the specified byte offset.
template<class T> extern T read(const void*, std::size_t);
template<class T> extern void write(void*, std::size_t, T);
extern Ptr add(void*, std::size_t);
extern U32 atomic_increment_32(void*, std::size_t);
extern U32 atomic_decrement_32(void*, std::size_t); // returns previous value
extern void invoke_virtual_0(void*);
extern void invoke_virtual_8(void*, int);
extern Ptr invoke_virtual_150(void*);
extern void invoke_virtual_10(void*);
extern void invoke_virtual_38(void*);
extern void invoke_virtual_40(void*, void*);
extern void* memory_realloc(void*, std::size_t, unsigned);
extern void memory_free(void*);
extern void* native_vtable(std::uintptr_t);
extern void* native_code_address(std::uintptr_t);
extern void initialize_weak_reference(void* storage, void* object); // 0x142ee3b30
extern U64 capture_thread_or_context(); // 0x142bad740
extern void check_stack_cookie(); // 0x145803c70; prologue/epilogue ABI adapter

// Explicit representations of the decompiler-visible fields, not native layout.
struct MessageCopy {
  Byte kind;
  U64 value08;
  void* owner10;
  U32 value18;
  U64 value20;
  U32 delay28_bits;
  Byte value2c;
  U32 value30;
  U32 value34;
  U64 value38;
};
struct DelegateStorage { void* object; int count; };
struct TimerScratch {
  // Native weak reference and delegate storage overlap the outgoing call frame;
  // adapter must reconstruct their actual layout and initialized fields.
  alignas(8) Byte weak_reference[8];
  U64 zero_b0;
  U64 cleanup_marker;
  void* cleanup_object;
  alignas(8) Byte inline_cleanup[32];
  Byte option_low;
  U32 option_float_bits;
  U64 tutorial_request_arg; // meaningful only for tutorial scheduling
};
struct TutorialDelegateBuilder {
  void* callback_or_object;
  int count;
  U32 descriptor_low;
  U32 descriptor_one;
  U32 descriptor_zero;
  // The original copies an otherwise unknown stack word into descriptor's tail.
  alignas(4) Byte unknown_descriptor_tail[4];
};
extern void* copy_tutorial_message(void* opaque_storage, void* message); // 0x141981b60
extern DelegateStorage* build_tutorial_delegate(TutorialDelegateBuilder*, void* game_state,
                                                void* descriptor, void* message_copy); // 0x14197b750
extern void schedule_timer(void* manager, void* handle, DelegateStorage*, U32 delay_bits,
                           TimerScratch&); // 0x1446278d0: native extra stack arguments unresolved
extern void enqueue_tutorial(void* queue, const MessageCopy&); // 0x14198a860
extern void enqueue_game_mode(void* queue, const MessageCopy&); // 0x14198aa30

static void retain(void* owner) {
  if (owner) atomic_increment_32(owner, 8);
}
static void release(void* owner) {
  if (owner && atomic_decrement_32(owner, 8) == 1) {
    invoke_virtual_0(owner);
    if (atomic_decrement_32(owner, 12) == 1) invoke_virtual_8(owner, 1);
  }
}
static MessageCopy copy_message(void* message, float initial_delay) {
  MessageCopy c;
  c.kind = read<Byte>(message, 0);
  c.value08 = read<U64>(message, 8);
  c.owner10 = read<void*>(message, 0x10);
  // The original rereads delay only when a refcount increment occurred.
  if (c.owner10) { retain(c.owner10); initial_delay = read<float>(message, 0x28); }
  c.value18 = read<U32>(message, 0x18);
  c.value34 = read<U32>(message, 0x34);
  c.value20 = read<U64>(message, 0x20);
  c.value2c = read<Byte>(message, 0x2c);
  c.value30 = read<U32>(message, 0x30);
  c.value38 = read<U64>(message, 0x38);
  // Preserve the exact float bits without numeric conversion.
  union { float f; U32 u; } bits; bits.f = initial_delay; c.delay28_bits = bits.u;
  return c;
}
static void* timer_manager(void* context) {
  if (!read<void*>(context, 0x188)) return read<void*>(context, 0x320);
  return read<void*>(read<void*>(context, 0x188), 0xc0);
}
static void init_timer_scratch(TimerScratch& scratch) {
  initialize_weak_reference(scratch.weak_reference, nullptr);
  scratch.option_float_bits = 0xbf800000;
  scratch.option_low = 0;
  scratch.zero_b0 = 0;
  scratch.cleanup_marker = 0;
  scratch.cleanup_object = nullptr;
}
static void cleanup_timer_scratch(TimerScratch& scratch) {
  if (scratch.cleanup_marker)
    invoke_virtual_10(scratch.cleanup_object ? scratch.cleanup_object : scratch.inline_cleanup);
}
// Reproduce the original count/null branches, including the count-nonzero,
// null-object path that skips freeing the object pointer.
static void cleanup_delegate(DelegateStorage& d) {
  void* object = d.count ? d.object : nullptr;
  if (!object) {
    if (!d.count) { if (d.object) memory_free(d.object); return; }
    if (!d.object) return;
    object = d.object;
  }
  invoke_virtual_38(object);
  if (d.object) d.object = memory_realloc(d.object, 0, 0);
  d.count = 0;
  if (d.object) memory_free(d.object);
}

void HandleContextTutorialGameModeMessageRequest(void* game_state, void* message, U64 request_arg) {
  float delay = read<float>(message, 0x28);
  if (!(delay < 0.0f)) {
    if (1e-8 < std::fabs(delay)) {
      void* context = invoke_virtual_150(game_state);
      if (context) {
        TutorialDelegateBuilder builder;
        builder.callback_or_object = native_code_address(0x14198abf0);
        builder.count = 0;
        builder.descriptor_low = 0x4198abf0;
        builder.descriptor_one = 1;
        builder.descriptor_zero = 0;
        void* manager = timer_manager(context);
        U32 duration = read<U32>(message, 0x28);
        alignas(8) Byte copied_message_storage[64];
        void* copied_message = copy_tutorial_message(copied_message_storage, message);
        DelegateStorage* built = build_tutorial_delegate(&builder, game_state,
                                                         &builder.descriptor_low, copied_message);
        DelegateStorage delegate{nullptr, 0};
        if (built != &delegate && built->count && built->object)
          invoke_virtual_40(built->object, &delegate);
        TimerScratch scratch; init_timer_scratch(scratch);
        // Only the low byte is cleared; remaining request_arg bits are retained.
        scratch.tutorial_request_arg = request_arg & ~U64{0xff};
        schedule_timer(manager, add(game_state, 0x2938), &delegate, duration, scratch);
        cleanup_timer_scratch(scratch);
        cleanup_delegate(delegate);
        if (builder.count) {
          if (!builder.callback_or_object) goto tutorial_done;
          invoke_virtual_38(builder.callback_or_object);
          if (builder.callback_or_object)
            builder.callback_or_object = memory_realloc(builder.callback_or_object, 0, 0);
          builder.count = 0;
        }
        if (builder.callback_or_object) memory_free(builder.callback_or_object);
      }
    } else {
      MessageCopy copy = copy_message(message, delay);
      enqueue_tutorial(add(game_state, 0x10c8), copy);
    }
  }
tutorial_done:
  release(read<void*>(message, 0x10));
  check_stack_cookie();
}

void HandleGameModeMessageRequest(void* game_state, void* message) {
  float delay = read<float>(message, 0x28);
  if (!(delay < 0.0f)) {
    if (1e-8 < std::fabs(delay)) {
      void* context = invoke_virtual_150(game_state);
      if (context) {
        // Unlike the near-zero path, delay is reread unconditionally after retain.
        Byte kind = read<Byte>(message, 0);
        void* owner = read<void*>(message, 0x10);
        U64 value08 = read<U64>(message, 8);
        U64 timer_handle = 0;
        retain(owner);
        U32 value18 = read<U32>(message, 0x18);
        U32 duration_copy = read<U32>(message, 0x28);
        U32 value34 = read<U32>(message, 0x34);
        U64 value20 = read<U64>(message, 0x20);
        Byte value2c = read<Byte>(message, 0x2c);
        U32 value30 = read<U32>(message, 0x30);
        U64 value38 = read<U64>(message, 0x38);
        void* callback = memory_realloc(nullptr, 0x70, 0);
        if (callback) {
          write<void*>(callback, 0, native_vtable(0x145e13800));
          initialize_weak_reference(add(callback, 8), game_state);
          write<Byte>(callback, 0x18, kind);
          write<U64>(callback, 0x20, value08);
          write<void*>(callback, 0x10, game_state);
          write<void*>(callback, 0x28, owner);
          retain(owner);
          write<U32>(callback, 0x30, value18);
          write<U32>(callback, 0x40, duration_copy);
          write<U32>(callback, 0x4c, value34);
          write<U64>(callback, 0x38, value20);
          write<Byte>(callback, 0x44, value2c);
          write<U32>(callback, 0x48, value30);
          write<U64>(callback, 0x50, value38);
          write<U64>(callback, 0x60, capture_thread_or_context());
          write<void*>(callback, 0, native_vtable(0x145e13858));
        }
        void* manager = timer_manager(context);
        U32 duration = read<U32>(message, 0x28);
        DelegateStorage delegate{nullptr, 0};
        if (callback) invoke_virtual_40(callback, &delegate);
        TimerScratch scratch; init_timer_scratch(scratch);
        schedule_timer(manager, &timer_handle, &delegate, duration, scratch);
        cleanup_timer_scratch(scratch);
        cleanup_delegate(delegate);
        if (callback) {
          invoke_virtual_38(callback);
          callback = memory_realloc(callback, 0, 0);
          if (callback) memory_free(callback);
        }
        release(owner);
      }
    } else {
      MessageCopy copy = copy_message(message, delay);
      enqueue_game_mode(add(game_state, 0x10b8), copy);
    }
  }
  release(read<void*>(message, 0x10));
  check_stack_cookie();
}
} // namespace gamestate_native74
