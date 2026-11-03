#pragma once
#include <cstdint>
namespace death72 {
struct Object; struct World; struct DeathInfo; struct String;
// Each declaration is one unresolved field access, engine call, or stack value.
// The implementation must preserve engine ABI and each operation's ownership.
struct MarkerEngine {
  int32_t read_i32(Object*, unsigned); void write_i32(Object*, unsigned, int32_t);
  uint64_t read_u64(const void*, unsigned); uint32_t read_u32(const void*, unsigned);
  float read_float(Object*, unsigned); void* read_pointer(Object*, unsigned);
  void* marker_array_grow_14069e9b0(Object* game_state, int32_t old_count);
  void write_marker_record(void* array, int32_t index, uint64_t word0, uint64_t word1,
                           uint32_t word2, uint32_t word3, uint32_t word4, uint32_t word5);
  // Upper six bytes of uStack_8d0 are carried from prior stack state, not initialized here.
  uint64_t prior_stack_8d0();
  uint64_t dat_148218630(); uint32_t dat_148218638();
  float dat_147c31cb0(); float dat_147c31cb4(); float dat_147c31cb8();
  void init_141a58ab0(void*); void init_141a0a3b0(void*);
  // Returned temporary is an FText-like object, not necessarily a real C++ struct.
  void* text_from_string_142c26f20(void* temporary, String*);
  uint64_t text_word(void* text, unsigned word); uint32_t text_flags(void* text);
  void set_text_word(void* text, unsigned word, uint64_t);
  void destroy_text_140683490(void* temporary);
  void* registry_141a777c0(); void* lookup_141a78fd0(void* registry, Object** game_state_address);
  void apply_141a8c520(void* payload, void* secondary);
  void* package_141683c20(void* scratch, void* payload);
  void send_141a8f510(void* sink, Object** controller_address, void* packaged);
  void destroy_secondary_141a32c10(void* secondary);
  void destroy_payload_1416854c0(void* payload);
};
void marker_append_and_emit(MarkerEngine&, Object* game_state, World* current_world,
                            DeathInfo*, Object* controller, String* victim_name,
                            uint8_t victim_squad, uint8_t victim_team);
}
