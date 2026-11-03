// Expanded lift of source lines ~1660-1733. Offsets below are relative to
// uStack_648, not a claimed native type layout. Only observed bytes are written.
#include "marker.rough.hpp"
#include <cstring>
#include <cstddef>
#include <cstdint>

namespace death72 {
namespace {
template<class T> void put(unsigned char* p, std::size_t offset, T value) {
  std::memcpy(p + offset, &value, sizeof value);
}
template<class T> T get(unsigned char* p, std::size_t offset) {
  T value; std::memcpy(&value, p + offset, sizeof value); return value;
}
}
void marker_append_and_emit(MarkerEngine& e, Object* game_state, World* current_world,
                            DeathInfo* info, Object* controller, String* victim_name,
                            uint8_t victim_squad, uint8_t victim_team) {
  // Source dereferences game_state without a null guard. Increment count before grow.
  int32_t old_count=e.read_i32(game_state,0x2770);
  uint64_t position_xy=e.read_u64(info,0x18);
  uint32_t position_z=e.read_u32(info,0x20);
  uint32_t world_540=e.read_u32(current_world,0x540);
  e.write_i32(game_state,0x2770,old_count+1);
  uint64_t last=(e.prior_stack_8d0() & UINT64_C(0xffffffffffff0000)) |
                uint64_t(victim_squad) | (uint64_t(victim_team)<<8);
  if (e.read_i32(game_state,0x2774)<old_count+1)
    e.marker_array_grow_14069e9b0(game_state,old_count);
  void* data=e.read_pointer(game_state,0x2768);
  e.write_marker_record(data,old_count,uint64_t(reinterpret_cast<uintptr_t>(game_state)),
                        position_xy,position_z,world_540,uint32_t(last),uint32_t(last>>32));
  void* sink=e.read_pointer(controller,0x3618);
  if (!sink) return;

  // Raw stack object; unwritten bytes intentionally remain unspecified.
  alignas(8) unsigned char payload[0x100]; // stack 648 through 549
  put(payload,0x00,UINT64_MAX); put(payload,0x08,UINT64_MAX);
  put(payload,0x24,e.dat_148218630()); // 624
  put(payload,0x30,e.dat_148218630()); // 618
  put(payload,0x3c,e.dat_147c31cb0()); put(payload,0x40,e.dat_147c31cb4());
  put(payload,0x2c,e.dat_148218638());
  // Decompiler casts DAT_148218638 to float here; representation needs validation.
  put(payload,0x38,static_cast<float>(e.dat_148218638()));
  put(payload,0x10,UINT32_MAX); put(payload,0x18,UINT64_C(0)); put(payload,0x20,uint8_t(0));
  put(payload,0x44,e.dat_147c31cb8()); put(payload,0x48,UINT64_C(0));
  put(payload,0x50,uint32_t(1)); put(payload,0x58,UINT64_C(0));
  put(payload,0x70,UINT64_C(0)); put(payload,0x78,UINT64_C(0)); put(payload,0x80,UINT64_C(0));
  put(payload,0x60,UINT32_MAX); put(payload,0x64,uint32_t(0));
  e.init_141a58ab0(payload+0x70);
  put(payload,0x68,uint32_t(0)); put(payload,0x88,uint16_t(1)); put(payload,0x8a,uint8_t(1));
  e.init_141a0a3b0(payload+0x90);
  float scale=e.read_float(game_state,0x2764);
  put(payload,0x24,e.read_u64(info,0x18)); put(payload,0x2c,e.read_u32(info,0x20));
  float z=e.dat_147c31cb8()*scale;
  put(payload,0xf8,UINT64_C(0)); // stack 550
  put(payload,0x20,uint8_t(4)); put(payload,0x8a,uint8_t(0));
  put(payload,0x30,e.dat_147c31cb0()*scale); put(payload,0x34,e.dat_147c31cb4()*scale);
  put(payload,0x38,z);

  alignas(8) unsigned char secondary[0x80]; // stack 508 through >= 4c8
  alignas(8) unsigned char text_temporary[24];
  alignas(8) unsigned char package_scratch[256];
  e.init_141a0a3b0(secondary);
  void* temp=e.text_from_string_142c26f20(text_temporary,victim_name);
  uint64_t old0=get<uint64_t>(secondary,0), old1=get<uint64_t>(secondary,8);
  uint64_t new0=e.text_word(temp,0), new1=e.text_word(temp,1);
  e.set_text_word(temp,0,old0); e.set_text_word(temp,1,old1);
  put(secondary,0,new0); put(secondary,8,new1); put(secondary,0x10,e.text_flags(temp));
  e.destroy_text_140683490(text_temporary); // releases the swapped-out old text
  void* registry=e.registry_141a777c0();
  void* registry_entry=e.lookup_141a78fd0(registry,&game_state);
  put(secondary,0x40,e.read_u64(registry_entry,0));
  e.apply_141a8c520(payload,secondary);
  void* packaged=e.package_141683c20(package_scratch,payload);
  e.send_141a8f510(sink,&controller,packaged);
  e.destroy_secondary_141a32c10(secondary);
  e.destroy_payload_1416854c0(payload);
}
}
