// Source recovery draft for 0x14110c030. Adapters describe unresolved ABI/layout,
// allocator and individual callee effects; they are not engine implementations.
#include <cstdint>
namespace profile_reset81 {
using i32 = int32_t; using u32 = uint32_t; using byte = uint8_t;
struct Shared { void* object; void* control; };
struct Sparse; struct Frame; struct StringAllocation; struct ReplayRecord;
extern i32 read_i32(void*, unsigned); extern void* read_ptr(void*, unsigned);
extern byte read_byte(void*, unsigned); extern void write_i32(void*, unsigned, i32);
extern void write_byte(void*, unsigned, byte); extern void* offset(void*, unsigned);
extern void retain(void*); // non-atomic strong +1 at control+8
extern void release(void*); // strong -1; zero: virtual 0, weak -1, zero: virtual +8(control,1)
extern i32 handle_bucket(void* backing, u32 hash); // inline/heap buckets +280/+288
extern i32 handle_key(void* backing, i32 slot); extern i32 handle_next(void*, i32);
extern void remove_handle_14110acd0(void* map248, i32* handle);
extern void* weak_get_140(void* weak17c);
extern bool log_enabled(i32 level); extern void log_handle(i32 handle, void* owner);
// Builds the unset/formatted string with the native allocations/move/free sequence,
// logs at level 5, then frees it. No arguments missing from the recovered call are invented.
extern void log_141111020(Sparse*, void* entry); extern void log_literal(const wchar_t*);
extern void log_reapply_unresolved();
// Sparse cursor models the native inline-vs-heap bit words, high-set-bit selection,
// signed bounds, captured end and 0x140687d00 advancement. It retains identity
// of the inline bit storage and container in its end comparison.
struct Cursor { uintptr_t opaque[12]; };
extern Cursor sparse_begin(Sparse*); extern bool sparse_not_end(Cursor&, Sparse*);
extern void* sparse_slot(Cursor&, Sparse*, unsigned stride); extern void sparse_next_140687d00(Cursor&);
extern Sparse* overlay_map(void* backing); extern Sparse* frame_map(Frame*, unsigned which);
extern void dirty_allocate_1406842c0(void* set, void** allocation_out);
extern void dirty_copy_140682ce0(void* allocation, void* string_field);
extern u32 hash_140682990(i32 length, const void* text);
extern const void* empty_string_1459a4b30();
extern void dirty_insert_140686410(void* set, i32* out_index, u32 hash, void* allocation);
extern void* string_data(void* string); extern i32 string_count(void* string);
extern void allocation_hash_link_minus_one(void* allocation);
// Explicit frame initialization: only fields written by native are initialized.
extern Frame* frame_stack_storage(); extern void frame_initialize_observed(Frame*, void* owner, void* base8, i32 handle, void* arg3);
// 1410edd40 has an unrecovered return ABI and hidden arguments. Its returned map
// is moved into frame map 0 with 1410ee110 and the separately transferred buffer.
extern void* collect_1410edd40_unresolved(Frame*); extern void move_collected_1410ee110(Frame*, void* returned);
extern void remove_mocks_141102f30(Frame*);
extern i32 find_1410fa050(Sparse*, void* key); extern void* slot_at(Sparse*, i32, unsigned stride);
extern Shared owner_lookup_virtual38(void* owner, void* key_first_word);
extern void log_entry_1410f7300(void* key, void* lookup_object, void* entry10_or_null);
extern void* replay_keys_data(Frame*); extern i32 replay_keys_count(Frame*);
extern void* replay_groups_data(void* entry); extern i32 replay_groups_count(void* entry);
extern void* replay_records_data(void* group); extern i32 replay_records_count(void* group);
extern byte apply_1410f17b0(void** pair, void* record); // low byte only
extern void notify_141110eb0(void* owner, Shared* consumed, uint16_t flags);
extern void* output_alloc_1410f16a0(Sparse*); // allocator may return null; native subsequently dereferences
extern void copy_state_1410ec6a0(void* dest, void* src);
extern void output_insert_1410f7c80(Sparse*, i32* out_index, u32 hash, void* allocation);
extern void reset_140d32350(void* backing1f8);
extern void callback_1410fa730(void* record, void* function_1410f5220, void* arg3_address);
extern void* function_1410f5220();
extern void remove_batch_14110b3a0(void* batches, i32 index, i32 one1, i32 one2);
// Destructors below release elements and free precisely the storage indicated;
// individual frees are separate to preserve observed order.
extern void free_buffer(void*); extern void destroy_140a2ed30(Frame*, i32);
extern void destroy_140a2f240(Frame*, unsigned map, i32); extern void destroy_1410f87b0(Frame*, i32);
extern void* frame_buffer(Frame*, unsigned offset); extern void frame_zero32(Frame*, unsigned offset);
extern uint16_t read_u16(void*, unsigned);
static void dirty(void* set, void* field, i32* index) {
  void* allocation; dirty_allocate_1406842c0(set, &allocation);
  if (allocation) { dirty_copy_140682ce0(allocation, field); allocation_hash_link_minus_one(allocation); }
  // Native dereferences even if allocator returned null.
  i32 count = string_count(allocation); i32 n = count ? count - 1 : 0;
  const void* text = count ? string_data(allocation) : empty_string_1459a4b30();
  u32 hash = hash_140682990(n, text);
  dirty_insert_140686410(set, index, hash, allocation);
}
void reset_14110c030(void* backing, i32 handle, void* arg3) {
  i32 scratch274 = 0, scratch278; // 278 has no initial value in the source
  if (read_i32(backing, 0x250) == read_i32(backing, 0x27c)) return;
  u32 mask = u32(read_i32(backing, 0x290)) - 1u;
  i32 slot = handle_bucket(backing, mask & u32(handle));
  while (slot != -1 && handle_key(backing, slot) != handle) slot = handle_next(backing, slot);
  if (slot == -1) return;
  remove_handle_14110acd0(offset(backing, 0x248), &handle);
  void* owner = weak_get_140(offset(backing, 0x17c));
  if (!owner) return;
  if (log_enabled(5)) log_handle(handle, owner);
  Sparse* overlays = overlay_map(backing);
  for (Cursor c = sparse_begin(overlays); sparse_not_end(c, overlays); sparse_next_140687d00(c)) {
    void* entry = sparse_slot(c, overlays, 0xe8);
    void* object = read_ptr(entry, 0x10);
    dirty(offset(backing, 0x2e8), object, &scratch274);
    object = read_ptr(entry, 0x10);
    dirty(offset(backing, 0x2e8), offset(object, 0x20), &scratch278);
  }
  Frame* f = frame_stack_storage();
  frame_initialize_observed(f, owner, offset(backing, 8), handle, arg3);
  void* collected = collect_1410edd40_unresolved(f);
  move_collected_1410ee110(f, collected); // includes identity check and buffer transfer
  Sparse* map = frame_map(f, 0);
  if (log_enabled(6)) {
    log_literal(L"Pre-removal mocks:");
    for (Cursor c = sparse_begin(map); sparse_not_end(c, map); sparse_next_140687d00(c))
      log_141111020(map, sparse_slot(c, map, 0xe8));
  }
  remove_mocks_141102f30(f);
  if (log_enabled(6)) {
    log_literal(L"Post-removal mocks:");
    for (Cursor c = sparse_begin(map); sparse_not_end(c, map); sparse_next_140687d00(c)) {
      void* key = sparse_slot(c, map, 0xe8);
      if (log_enabled(6)) {
        i32 found = find_1410fa050(map, key);
        void* e = found == -1 ? nullptr : slot_at(map, found, 0xe8);
        Shared result = owner_lookup_virtual38(owner, read_ptr(key, 0));
        log_entry_1410f7300(key, result.object, e ? offset(e, 0x10) : nullptr);
        if (result.control) release(result.control);
      }
    }
  }
  if (replay_keys_count(f) != 0 && log_enabled(6)) log_literal(L"Reapplying mocks to: %d items");
  byte* keys = static_cast<byte*>(replay_keys_data(f));
  byte* keys_end = keys + int64_t(replay_keys_count(f)) * 16;
  for (; keys != keys_end; keys += 16) {
    i32 found = find_1410fa050(map, keys);
    void* e = found == -1 ? nullptr : slot_at(map, found, 0xe8);
    void* object = read_ptr(e, 0x10); // native dereference even when missing
    if (log_enabled(6)) {
      log_reapply_unresolved();
      if (log_enabled(6)) {
        i32 again = find_1410fa050(map, keys);
        void* logged = again == -1 ? nullptr : slot_at(map, again, 0xe8);
        Shared result = owner_lookup_virtual38(owner, read_ptr(keys, 0));
        log_entry_1410f7300(keys, result.object, logged ? offset(logged, 0x10) : nullptr);
        if (result.control) release(result.control);
      }
    }
    byte* groups = static_cast<byte*>(replay_groups_data(e));
    byte* groups_end = groups + int64_t(replay_groups_count(e)) * 0x18;
    for (; groups != groups_end; groups += 0x18) {
      byte* record = static_cast<byte*>(replay_records_data(groups));
      byte* end = record + int64_t(replay_records_count(groups)) * 0x110;
      for (; record != end; record += 0x110) {
        if (read_byte(record, 0x10c)) {
          i32 op = read_i32(record, 0), value = read_i32(record, 0x108);
          if (op == 0) write_i32(object, 0x30, read_i32(object, 0x30) + value);
          else if (op == 1) write_i32(object, 0x30, value);
          else if (op == 2) { i32 old = read_i32(object, 0x30); write_i32(object, 0x30, value <= old ? old : value); }
          write_byte(e, 0x20, 1);
        }
        void* pair[2] = {offset(object, 0x38), nullptr};
        if (apply_1410f17b0(pair, record)) write_byte(e, 0x21, 1);
      }
    }
    Shared updated{read_ptr(e, 0x10), read_ptr(e, 0x18)};
    if (updated.control) retain(updated.control);
    notify_141110eb0(owner, &updated, read_u16(e, 0x20));
  }
  Sparse* changed = frame_map(f, 1);
  for (Cursor c = sparse_begin(changed); sparse_not_end(c, changed); sparse_next_140687d00(c)) {
    void* item = sparse_slot(c, changed, 0x18);
    Shared key{read_ptr(item, 0), read_ptr(item, 8)};
    if (key.control) retain(key.control);
    i32 found = find_1410fa050(map, &key);
    void* e = found == -1 ? nullptr : slot_at(map, found, 0xe8);
    void* allocation = output_alloc_1410f16a0(overlays);
    if (allocation) {
      // Construct shared key and copy state at +0x10, including another retain.
      extern void write_ptr(void*, unsigned, void*);
      write_ptr(allocation, 0, key.object); write_ptr(allocation, 8, key.control);
      if (key.control) retain(key.control);
      copy_state_1410ec6a0(offset(allocation, 0x10), offset(e, 0x10));
      write_i32(allocation, 0xe0, -1);
    }
    // Native dereferences the allocator output even on failure.
    void* key_string = read_ptr(allocation, 0);
    i32 count = string_count(key_string), n = count ? count - 1 : 0;
    const void* text = count ? string_data(key_string) : empty_string_1459a4b30();
    u32 hash = hash_140682990(n, text);
    output_insert_1410f7c80(overlays, &scratch278, hash, allocation);
    if (key.control) release(key.control);
  }
  reset_140d32350(offset(backing, 0x1f8));
  for (i32 i = 0; i >= 0 && i < read_i32(backing, 0x1f0); ++i) {
    byte* batches = static_cast<byte*>(read_ptr(backing, 0x1e8));
    byte* batch = batches + int64_t(i) * 0x18;
    byte* record = static_cast<byte*>(read_ptr(batch, 8));
    byte* end = record + int64_t(read_i32(batch, 0x10)) * 0xf8;
    for (; record != end; record += 0xf8)
      callback_1410fa730(record, reinterpret_cast<void*>(&function_1410f5220), &arg3);
    if (read_i32(batch, 0) == handle) {
      remove_batch_14110b3a0(offset(backing, 0x1e8), i, 1, 1);
      --i;
    }
  }
  byte* batch = static_cast<byte*>(read_ptr(backing, 0x1e8));
  byte* batch_end = batch + int64_t(read_i32(backing, 0x1f0)) * 0x18;
  for (; batch != batch_end; batch += 0x18) {
    byte* record = static_cast<byte*>(read_ptr(batch, 8));
    byte* end = record + int64_t(read_i32(batch, 0x10)) * 0xf8;
    for (; record != end; record += 0xf8) {
      void* pair[2] = {offset(backing, 0xa8), offset(backing, 0x1f8)};
      apply_1410f17b0(pair, record);
    }
  }
  frame_zero32(f, 0x50); if (frame_buffer(f, 0x58)) free_buffer(frame_buffer(f, 0x58));
  destroy_140a2ed30(f, 0);
  if (frame_buffer(f, 0x78)) free_buffer(frame_buffer(f, 0x78));
  if (frame_buffer(f, 0x98)) free_buffer(frame_buffer(f, 0x98));
  byte* refs = static_cast<byte*>(replay_keys_data(f));
  i32 remaining = replay_keys_count(f);
  if (remaining != 0) do {
    void* control = read_ptr(refs, 8); if (control) release(control);
    refs += 16; --remaining;
  } while (remaining != 0);
  if (replay_keys_data(f)) free_buffer(replay_keys_data(f));
  frame_zero32(f, 0xd0); if (frame_buffer(f, 0xd8)) free_buffer(frame_buffer(f, 0xd8));
  destroy_140a2f240(f, 2, 0);
  if (frame_buffer(f, 0xf8)) free_buffer(frame_buffer(f, 0xf8));
  if (frame_buffer(f, 0x118)) free_buffer(frame_buffer(f, 0x118));
  frame_zero32(f, 0x120); if (frame_buffer(f, 0x128)) free_buffer(frame_buffer(f, 0x128));
  destroy_140a2f240(f, 1, 0);
  if (frame_buffer(f, 0x148)) free_buffer(frame_buffer(f, 0x148));
  if (frame_buffer(f, 0x168)) free_buffer(frame_buffer(f, 0x168));
  frame_zero32(f, 0x170); if (frame_buffer(f, 0x178)) free_buffer(frame_buffer(f, 0x178));
  destroy_1410f87b0(f, 0);
  if (frame_buffer(f, 0x198)) free_buffer(frame_buffer(f, 0x198));
  if (frame_buffer(f, 0x1b8)) free_buffer(frame_buffer(f, 0x1b8));
}
}
