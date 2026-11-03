// Uncompiled, 64-bit ABI sketch of 0x141e4f8a0; see HANDOFF.md.
#include <cstdint>
#include <cstring>
namespace scoreboard_native87 {
using P = void*;
using U8 = uint8_t;
using U32 = uint32_t;
using U64 = uint64_t;
struct StringViewABI { const char16_t* data; int32_t count, capacity; };
struct TaskTriple { P task; P arg; U32 index; U32 pad; };
template<class T> static T& field(P p, unsigned offset) { return *reinterpret_cast<T*>(static_cast<U8*>(p) + offset); }
extern U8 trace_disabled_1482472e4, log_verbosity_14812df40;
extern U8 log_category_14812df44;
extern const char16_t empty_1459a4b30[], format_1459a4c8c[];
extern U8 closure_vtable_145ffc0f0;
extern void callback_141e51ca0();
extern void append_141e100c0(P destination, const char16_t* text);
extern P runtime_options_1426fdde0();
extern void flush_141df4ff0(P destination);
extern TaskTriple* make_task_140d86190(P output24, int zero, int ff);
extern void dispatch_140dac6b0(P task, P arg, U32 index, bool one);
extern void release_task_142bb0f40(P);
extern void close_trace_142cd3770(const char16_t**);
extern void log_142c54b70(P file, int line, P category, int verbosity, const char16_t* format, const char16_t* text);
extern void invoke_destroy_vtable_10(P object); // indirect virtual slot +0x10
extern void invoke_destroy_vtable_08(P object, int deleting); // indirect slot +8

static const char16_t* text(const StringViewABI* s) { return s->count != 0 ? s->data : empty_1459a4b30; }
static void release_task_ref(P ref) {
  if (ref && __atomic_fetch_sub(&field<U32>(ref, 0x48), 1u, __ATOMIC_SEQ_CST) == 1)
    release_task_142bb0f40(ref);
}
void add_scoreboard_entry(P self, const StringViewABI* value) {
  const char16_t* outer = trace_disabled_1482472e4 ? nullptr : u"STAT_AddScoreboardInfoEntry";
  P destination = field<P>(self, 0x7f8);
  if (destination) {
    append_141e100c0(destination, text(value));
    P options = runtime_options_1426fdde0();
    if (!field<U8>(options, 0x838)) {
      const char16_t* scope = trace_disabled_1482472e4 ? nullptr : u"STAT_AddScoreboardInfoEntry_Flush";
      flush_141df4ff0(field<P>(self, 0x7f8));
      if (trace_disabled_1482472e4 && scope) close_trace_142cd3770(&scope);
    } else {
      const char16_t* scope = trace_disabled_1482472e4 ? nullptr : u"STAT_AddScoreboardInfoEntry_AsyncFlush";
      P* current = &field<P>(self, 0x3f8);
      P previous = *current;
      U64 flags = 0;
      if (previous) {
        // LOCK CMPXCHG [previous+8], 0 with expected zero: an atomic read
        // with a zero write only when already zero.
        U64 expected = 0;
        __atomic_compare_exchange_n(&field<U64>(previous, 8), &expected, 0,
                                    false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
        flags = expected;
      }
      if (!previous || ((flags >> 26) & 1)) {
        P captured = field<P>(self, 0x7f8);
        P owner = field<P>(self, 0x800);
        if (owner) { ++field<U32>(owner, 0xc); ++field<U32>(owner, 0xc); }
        P closure_external = nullptr;
        P closure_function = reinterpret_cast<P>(&callback_141e51ca0);
        // Source copies 32 bytes; its final eight stack bytes are not initialized
        // by this function and must not be silently replaced with zero.
        alignas(16) U8 closure_payload[32];
        P vtable = &closure_vtable_145ffc0f0;
        std::memcpy(closure_payload, &vtable, 8);
        std::memcpy(closure_payload + 8, &captured, 8);
        std::memcpy(closure_payload + 16, &owner, 8);
        alignas(8) U8 task_result[24];
        TaskTriple* triple = make_task_140d86190(task_result, 0, 0xff);
        P task = triple->task;
        field<P>(task, 0x10) = closure_function;
        field<P>(task, 0x20) = closure_external;
        std::memcpy(static_cast<U8*>(task) + 0x30, closure_payload, 32);
        if (field<P>(task, 0x10)) closure_function = nullptr;
        field<U32>(task, 0x50) = 2;
        P new_ref = field<P>(task, 0x68);
        if (new_ref) __atomic_fetch_add(&field<U32>(new_ref, 0x48), 1u, __ATOMIC_SEQ_CST);
        dispatch_140dac6b0(task, triple->arg, triple->index, true);
        if (closure_function) invoke_destroy_vtable_10(closure_external ? closure_external : closure_payload);
        if (current == &new_ref) release_task_ref(new_ref);
        else { P old_ref = *current; *current = new_ref; release_task_ref(old_ref); }
        if (owner && --field<U32>(owner, 0xc) == 0) invoke_destroy_vtable_08(owner, 1);
      }
      if (trace_disabled_1482472e4 && scope) close_trace_142cd3770(&scope);
    }
  }
  if (log_verbosity_14812df40 >= 7)
    log_142c54b70(nullptr, 0, &log_category_14812df44, 7, format_1459a4c8c, text(value));
  if (trace_disabled_1482472e4 && outer) close_trace_142cd3770(&outer);
}
}
