// Uncompiled behavioral lift of exec 0x142a7add0. See HANDOFF.md.
#include <cstdint>
#include <cstring>

namespace hoverboard_wrap87 {
using P = void*;
using I32 = int32_t;
using U64 = uint64_t;
struct Array { P data; I32 count, capacity; };
// Original 0x48-byte record: four scalar words, two owned tag-container
// arrays, object pointer, and a trailing owned 16-byte value.
struct Record { uint32_t words[4]; uint8_t tags[32]; P object; uint8_t tail[16]; };
static_assert(sizeof(Record) == 0x48, "record layout");
// Observed stack slots, including unwritten gaps and the trailing zero word.
struct WrapContext {
  Array a00; Array a10; U64 words20[3]; uint8_t byte38, gap39[7]; U64 word40;
  Array records; uint8_t byte58, gap59[7]; U64 words60[7];
  Array a98; U64 wordsa8[3]; Array ac0;
  U64 wordd0; Array ad8; U64 worde8;
};
static_assert(sizeof(WrapContext) == 0xf0, "local layout");
static_assert(__builtin_offsetof(WrapContext, records) == 0x48, "records offset");
static void initialize_context(WrapContext& c) {
  c.a00 = {}; c.a10 = {};
  for (U64& w : c.words20) w = 0;
  c.byte38 = 0; c.word40 = 0; c.records = {}; c.byte58 = 0;
  for (U64& w : c.words60) w = 0;
  c.a98 = {}; for (U64& w : c.wordsa8) w = 0;
  c.ac0 = {}; c.wordd0 = 0; c.ad8 = {}; c.worde8 = 0;
  // Native writes only one byte at 0x38 and 0x58; leave adjacent gaps unwritten.
}
// Output is zeroed except its first word. Its final pointer and the pointer at
// offset 8 are freed following the virtual call.
struct CallOutput { uint32_t first, second; Array a08; uint8_t flag; uint8_t pad[7]; Array a20; };
extern P attachment_mesh(P pawn); // Context->AttachmentMeshComponent
extern bool loadout_initialized(P pawn); // CosmeticLoadout[0xf0]
extern P virtual_548(P mesh, int index);
extern bool virtual_450(P material);
extern P loadout_source_142323510(P pawn, int zero);
extern void context_assign_140a003b0(WrapContext*, P);
extern P find_wrap_141d4abb0(WrapContext*, int zero);
extern P pawn_field_250(P pawn); // Context + 0x250; exact field ABI unresolved
extern P class_142a3d9f0();
extern P object_class(P); extern I32 class_depth(P); extern P class_chain_entry(P, I32);
extern P class_chain_address(P); // class + 0x30
extern void gather_141d50e00(WrapContext*, P wrap, Array* out);
extern P select_141d85c80(P wrap, int mode, Array* variants, P target);
extern P virtual_568(P mesh, int index, P material);
extern void copy_material_144335300(P dynamic_material, P old_material, int one);
extern void grow_1407f2320(Array* destination); // source-specific growth ABI unresolved
extern void tag_copy_143e8a690(P destination, const void* source);
extern void tail_copy_140682ce0(P destination, const void* source);
extern void virtual_4e0(P wrap, int zero, WrapContext*, CallOutput*);
extern void free_memory(P);
extern bool log_level_at_least_3();
extern void log_uninitialized_loadout(P pawn, P mesh); // unresolved name formatting + logging

static void release_records(Array& array) {
  auto* records = static_cast<Record*>(array.data);
  for (I32 i = 0; i != array.count; ++i) {
    P p;
    std::memcpy(&p, records[i].tail, sizeof(p)); if (p) free_memory(p);
    std::memcpy(&p, records[i].tags + 16, sizeof(p)); if (p) free_memory(p);
    std::memcpy(&p, records[i].tags, sizeof(p)); if (p) free_memory(p);
  }
  if (array.data) free_memory(array.data);
}
static void release_context(WrapContext& context) {
  if (context.ad8.data) free_memory(context.ad8.data);
  if (context.ac0.data) free_memory(context.ac0.data);
  if (context.a98.data) free_memory(context.a98.data);
  release_records(context.records);
  if (context.a10.data) free_memory(context.a10.data);
  if (context.a00.data) free_memory(context.a00.data);
}
void apply(P pawn) {
  // P_FINISH is performed by the exec wrapper. The native reads pawn before
  // the inlined helper's null-pawn diagnostic, making that branch unreachable.
  P mesh = attachment_mesh(pawn);
  if (!mesh) return;
  P old_material = virtual_548(mesh, 0);
  const int mode = old_material && virtual_450(old_material) ? 2 : 1;
  WrapContext context; initialize_context(context);
  if (!loadout_initialized(pawn)) {
    if (log_level_at_least_3()) log_uninitialized_loadout(pawn, mesh);
    release_context(context);
    return;
  }
  context_assign_140a003b0(&context, loadout_source_142323510(pawn, 0));
  Array variants{};
  P wrap = find_wrap_141d4abb0(&context, 0);
  if (wrap) {
    P target = pawn_field_250(pawn);
    if (target) {
      P requested = class_142a3d9f0();
      P actual = object_class(target);
      I32 depth = class_depth(requested);
      if (class_depth(actual) < depth || class_chain_entry(actual, depth) != class_chain_address(requested)) target = nullptr;
    }
    gather_141d50e00(&context, wrap, &variants);
    P selected = select_141d85c80(wrap, mode, &variants, target);
    if (selected && selected != old_material) {
      P dynamic_material = virtual_568(mesh, 0, selected);
      if (dynamic_material) copy_material_144335300(dynamic_material, old_material, 1);
    }
    const I32 count = variants.count;
    if (count != 0) {
      if (context.records.capacity < context.records.count + count) grow_1407f2320(&context.records);
      auto* src = static_cast<Record*>(variants.data);
      auto* dst = static_cast<Record*>(context.records.data) + context.records.count;
      for (I32 i = 0; i != count; ++i) {
        for (int w = 0; w < 4; ++w) dst[i].words[w] = src[i].words[w];
        std::memset(dst[i].tags, 0, sizeof(dst[i].tags));
        tag_copy_143e8a690(dst[i].tags, src[i].tags);
        dst[i].object = src[i].object;
        tail_copy_140682ce0(dst[i].tail, src[i].tail);
      }
      context.records.count += count;
    }
    CallOutput output{}; output.first = 0xffffffffu;
    virtual_4e0(wrap, 0, &context, &output);
    if (output.a20.data) free_memory(output.a20.data);
    if (output.a08.data) free_memory(output.a08.data);
  }
  release_records(variants);
  release_context(context);
}
}
