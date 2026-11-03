// Uncounted behavioral lift of 0x1419e9980. All externs are unresolved native contracts.
#include <atomic>
#include <cstdint>
namespace rename_callback77 {
struct Controller; struct Plot; struct Identity; struct FormatPattern;
struct String { void* data; int32_t num, max; };
struct AtomicControl { void** vtable; std::atomic<int32_t> strong, weak; };
struct LocalControl { void** vtable; int32_t strong, weak; };
struct Text { uint64_t value; AtomicControl* control; uint32_t flags; };
struct IdentityTemp { void* vtable; Identity* identity; LocalControl* control; void* allocation; };
struct Entry { String key; uint32_t kind; uint32_t pad; uint64_t unknown; Text text; uint8_t has_text; uint8_t pad2[7]; int32_t hash_next; uint32_t pad3; };
struct Sparse {
  Entry* entries; int32_t count, capacity;
  uint32_t inline_bits[4]; uint32_t* heap_bits; int32_t bit_count, bit_capacity;
  int32_t free_head, free_count; void* hash; uint32_t hash_state;
};
extern void free_mem(void*); extern void destroy_atomic(AtomicControl*); extern void delete_atomic(AtomicControl*, int);
extern void destroy_local(LocalControl*); extern void delete_local(LocalControl*, int);
extern void* empty_wide_string(); extern void* FUN_140d72ee0(char*, String&);
extern Text* FUN_142c26df0(Text&, void*); extern void reserve_string(String&, int32_t);
extern void grow_string(String&); extern void copy_bytes(void*, const void*, int64_t);
extern void grow_entries(Sparse&, int32_t old_count); extern uint64_t quantize_size(uint64_t, uint32_t);
extern void resize_bits(Sparse&, int32_t old_count); extern uint32_t hash_string(const void*, int);
extern void FUN_1407babb0(Sparse&, void* scratch, uint32_t, Entry*);
extern void destroy_sparse(Sparse&); // native FUN_140799530, not an omitted inline cleanup
extern void delete_atomic_without_flag(AtomicControl*); extern uint64_t unresolved_stack_70();
extern void unresolved_initial_inline_bits(uint32_t (&)[4]);
extern Text make_text(const wchar_t*, const wchar_t*, const wchar_t*); // FUN_142bedc40
extern FormatPattern& FUN_142c0c460(FormatPattern&, Text&);
extern Text& FUN_142c255f0(Text&, FormatPattern&, Sparse&);
extern FormatPattern& pattern_storage(); extern void* scratch_storage();
extern uint8_t log_verbosity(); extern void* log_category();
extern void* FUN_142c40540(Text&, /* native hidden/omitted args unresolved */ void*);
extern void* unresolved_log_prepare_args(); extern void* unresolved_log_variadic_arg();
extern void logf(void*, int, void*, int, const wchar_t*, void*);
extern void FUN_142a52b90(Controller*, int, Text&);
extern void* controller_field_3528(Controller*); extern void* FUN_1418c6640(void* omitted_args); extern void* unresolved_1418c6640_args();
extern void* FUN_14297b320(/* native omitted args */ void*); extern void* unresolved_14297b320_args();
extern Plot* virtual_610(void*, void*); extern void FUN_14237f3d0(Controller*, IdentityTemp&);
extern Identity* plot_identity_248(Plot*); extern LocalControl* plot_control_250(Plot*);
extern char identity_valid_28(Identity*); extern char identity_equal_00(Identity*, Identity*);
extern String& plot_string_290(Plot*); extern int FUN_142bcf780(const void*, const void*);
extern void FUN_141f02320(Plot*); extern void* identity_temp_destructor_vtable();

static void release_atomic(AtomicControl* c) {
  if (c && c->strong.fetch_sub(1) == 1) {
    destroy_atomic(c);
    if (c->weak.fetch_sub(1) == 1) delete_atomic(c, 1);
  }
}
static void release_format_atomic(AtomicControl* c) {
  if(c && c->strong.fetch_sub(1)==1) {
    destroy_atomic(c);
    if(c->weak.fetch_sub(1)==1) delete_atomic_without_flag(c);
  }
}
static void release_local(LocalControl* c) {
  if (c && --c->strong == 0) {
    destroy_local(c);
    if (--c->weak == 0) delete_local(c, 1);
  }
}
static uint32_t* bits(Sparse& s) { return s.heap_bits ? s.heap_bits : s.inline_bits; }
static int32_t word_index(int32_t i) { return (i + ((i >> 31) & 31)) >> 5; }

void FUN_1419e9980(Controller* self, char* result, String* title) {
  if (result[0] == 0 && result[0x40] != 0) {
    Sparse args{};
    args.bit_capacity = 0x80; args.free_head = -1;
    unresolved_initial_inline_bits(args.inline_bits);
    String reason_string{}; Text converted;
    void* reason = FUN_140d72ee0(result, reason_string);
    Text* source = FUN_142c26df0(converted, reason);
    uint64_t value = source->value;
    AtomicControl* control = source->control;
    if (control) control->strong.fetch_add(1);
    uint32_t flags = source->flags;
    String key{};
    reserve_string(key, 0x1d);
    key.num += 0x1d;
    if (key.max < key.num) grow_string(key);
    void* key_allocation = key.data;
    copy_bytes(key.data, L"MCPIslandRenameFailureReason", 0x3a);
    int32_t index = args.free_head;
    if (args.free_count == 0) {
      index = args.count;
      int32_t old_count = args.count++;
      if (args.capacity < args.count) grow_entries(args, old_count);
      int32_t old_bits = args.bit_count;
      if (args.bit_capacity < args.bit_count + 1) {
        uint32_t words = (args.bit_count + 0x20 + (((args.bit_count + 0x20) >> 31) & 31)) >> 5;
        if (words < 5) args.bit_capacity = 4;
        else {
          uint64_t wanted;
          if (((args.bit_capacity + 31 + (((args.bit_capacity + 31) >> 31) & 31)) >> 5) == 0)
            wanted = words > 1 ? words : 1;
          else wanted = (uint64_t(words) * 3 >> 3) + words;
          args.bit_capacity = int32_t(quantize_size(wanted * 4, 0) >> 2);
          if (args.bit_capacity < int32_t(words)) args.bit_capacity = 0x7fffffff;
        }
        args.bit_capacity = int32_t(uint32_t(args.bit_capacity) << 5);
        resize_bits(args, args.bit_count);
      }
      ++args.bit_count;
      bits(args)[word_index(old_bits)] &= ~(uint32_t(1) << (old_bits & 31));
    } else {
      --args.free_count;
      // Free-list next occupies offset 4 in the unused 0x48-byte slot.
      args.free_head = *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(args.entries) + int64_t(index) * 0x48 + 4);
      if (args.free_count != 0)
        *reinterpret_cast<int32_t*>(reinterpret_cast<char*>(args.entries) + int64_t(args.free_head) * 0x48) = -1;
    }
    bits(args)[word_index(index)] |= uint32_t(1) << (index & 31);
    Entry* entry = reinterpret_cast<Entry*>(reinterpret_cast<char*>(args.entries) + int64_t(index) * 0x48);
    if (entry) {
      entry->key = key;
      entry->unknown = unresolved_stack_70();
      entry->kind = 4; entry->has_text = 0;
      entry->text.value = value; entry->text.control = control;
      key.data = nullptr;
      if (control) control->strong.fetch_add(1);
      key_allocation = nullptr;
      entry->text.flags = flags; entry->has_text = 1; entry->hash_next = -1;
    }
    const void* key_chars = entry->key.num == 0 ? empty_wide_string() : entry->key.data;
    uint32_t hash = hash_string(key_chars, 0);
    FUN_1407babb0(args, scratch_storage(), hash, entry);
    if (key_allocation) free_mem(key_allocation);
    release_atomic(control);
    release_atomic(converted.control);
    if (reason_string.data) free_mem(reason_string.data);
    Text format = make_text(L"Reason: {MCPIslandRenameFailureReason}", L"Athena", L"IslandRenameMCPCallbackFailure");
    FormatPattern& pattern = FUN_142c0c460(pattern_storage(), format);
    Text formatted; FUN_142c255f0(formatted, pattern, args);
    release_format_atomic(format.control);
    if (log_verbosity() > 1) {
      FUN_142c40540(formatted, unresolved_log_prepare_args());
      logf(nullptr, 0, log_category(), 2, L"AFortPlayerControllerAthena::OnUpdateCreativePlotComplete - %s", unresolved_log_variadic_arg());
    }
    FUN_142a52b90(self, 0, formatted);
    release_atomic(formatted.control);
    args.hash_state = 0;
    if (args.hash) free_mem(args.hash);
    destroy_sparse(args);
  } else {
    Text complete = make_text(L"Complete", L"Athena", L"IslandRenameSuccess");
    if (log_verbosity() > 3) {
      FUN_142c40540(complete, unresolved_log_prepare_args());
      logf(nullptr, 0, log_category(), 4, L"AFortPlayerControllerAthena::OnUpdateCreativePlotComplete - %s", unresolved_log_variadic_arg());
    }
    if (controller_field_3528(self)) {
      void* provider = FUN_1418c6640(unresolved_1418c6640_args());
      if (provider) {
        void* selector = FUN_14297b320(unresolved_14297b320_args());
        Plot* plot = virtual_610(provider, selector);
        if (plot) {
          IdentityTemp current; FUN_14237f3d0(self, current);
          LocalControl* plot_control = plot_control_250(plot);
          Identity* plot_identity = plot_identity_248(plot);
          if (plot_control) ++plot_control->strong;
          bool a = plot_identity && identity_valid_28(plot_identity);
          bool b = current.identity && identity_valid_28(current.identity);
          bool same = a == b && (!a || identity_equal_00(plot_identity, current.identity));
          release_local(plot_control);
          if (same) {
            const void* requested = title->num == 0 ? empty_wide_string() : title->data;
            String& existing = plot_string_290(plot);
            const void* existing_chars = existing.num == 0 ? empty_wide_string() : existing.data;
            if (FUN_142bcf780(existing_chars, requested) == 0) FUN_141f02320(plot);
          }
          current.vtable = identity_temp_destructor_vtable();
          if (current.allocation) free_mem(current.allocation);
          release_local(current.control);
        }
      }
    }
    FUN_142a52b90(self, 1, complete);
    release_atomic(complete.control);
  }
  if (title->data) free_mem(title->data);
}
}
