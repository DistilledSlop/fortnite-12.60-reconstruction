// Draft behavioral transcription, not a drop-in Unreal implementation.
// Offsets identify original objects; adapter functions deliberately remain unresolved.
#include <cstdint>
#include <cstring>

struct Object;
struct Pawn;
struct Vec3 { float x, y, z; };
struct Quat { float x, y, z, w; };
struct TagContainer { void* tags; int32_t count, capacity; void* parents; int32_t parentCount, parentCapacity; };
struct Hits { void* data; int32_t count, capacity; };
struct Rotation { float x, y, z; };
extern uint8_t& byte_at(void*, unsigned);
extern float& float_at(void*, unsigned);
extern Object*& object_at(void*, unsigned);
extern void* ptr_at(void*, unsigned);
extern Vec3 load_vec3(void*, unsigned);
extern void store_vec3(void*, unsigned, Vec3);
extern void copy_bytes(void*, unsigned, void*, unsigned, unsigned);
extern bool is_a(Object*, uintptr_t static_class_helper);
extern Object* world(Pawn*); // vtable +0x150
extern Vec3 default_location(); // globals 0x148218630/638
extern bool trace_1440b59f0(Object*, Hits&, Vec3 end, Vec3 start); // remaining native flags 0,1 and global 0x148327520
extern Object* weak_hit_actor(Hits&, int32_t index); // stride 0x8c, weak ptr +0x74
extern float hit_z(Hits&, int32_t index); // hit +0x24
extern void free_hits(Hits&);
extern Object* call_142325510(Pawn*);
extern bool call_1423318f0(Pawn*);
extern void call_142a6f0a0(Object*);
extern float attribute_value(Object*, unsigned offset); // 0x14070cc00
extern void apply_proxy_attribute(Object*, uintptr_t descriptor_factory, float); // 0x141bc4d60; constructs/frees descriptor
extern void assign_montage_1406daaf0(Object*, Pawn*, unsigned source_offset);
extern void vcall_7d8(Object*);
extern void assign_tags(TagContainer&, const TagContainer&); // deep copy 0x143e8a690 / FGameplayTagContainer_CopyAssign
extern void destroy_tags(TagContainer&); // free both arrays, 0x142bee710
extern void call_141a0b170(Pawn*);
extern bool global_148114c00();
extern Rotation normalize_rotation_native(Rotation); // exact SIMD normalization 0x148115980 and thresholds
extern Quat rotation_to_quat(Rotation); // 0x142c5c060
extern void mesh_set_relative_location_rotation(Object*, Vec3, Quat); // 0x144122860, final args 0,0,0
extern void pawn_vcall_760(Pawn*, Rotation, Vec3);

void LandscapeCheck(Pawn* p) {
  byte_at(p,0x1063) &= 0xfe;
  float_at(p,0x106c) = -1.0f;
  if ((byte_at(p,0x58)&0x40) || *static_cast<int32_t*>(ptr_at(p,0x28d8)) != 0) return;
  Object* root = object_at(p,0x130);
  Vec3 start = root ? load_vec3(root,0x1d0) : default_location();
  Object* capsule = object_at(p,0x288);
  if (capsule) start.z += float_at(capsule,0x1e8)*float_at(capsule,0x400)*-0.5f;
  Vec3 end = start; end.z += 100000.0f;
  Hits hits{};
  Object* w = world(p);
  if (w && trace_1440b59f0(w,hits,end,start)) {
    for (int32_t i=0;i<hits.count;++i) {
      Object* actor = weak_hit_actor(hits,i);
      if (!actor || !is_a(actor,0x143afc5c0)) continue;
      byte_at(p,0x1063) |= 1;
      float_at(p,0x106c) = hit_z(hits,i)-start.z;
      root = object_at(p,0x130);
      store_vec3(p,0x1070,root ? load_vec3(root,0x1d0) : default_location());
      Object* related = call_142325510(p);
      if (related && !is_a(related,0x14285d610)) related=nullptr;
      if (call_1423318f0(p) && (!related || (byte_at(related,0x1063)&1) || !(byte_at(related,0x10e9)&0x20))) {
        Object* component=object_at(p,0x250);
        if (component && is_a(component,0x142a6ee20)) call_142a6f0a0(component);
      }
      break;
    }
  }
  free_hits(hits);
}

void OnRep_AttributeProxy(Pawn* p) {
  Object* movement=object_at(p,0xb90);
  if (!movement || byte_at(p,0xf0)!=1) return;
  constexpr uintptr_t factories[]={0x141a15a60,0x141a151e0,0x141a15620,0x141a13310,0x141a12bd0,0x141a12c80};
  for (unsigned i=0;i<6;++i) {
    // Reload MovementSet each iteration, as callbacks can mutate the pawn.
    movement=object_at(p,0xb90);
    float current=attribute_value(movement,0x30+i*0x28);
    float desired=float_at(p,0x2978+i*4);
    if (current!=desired) apply_proxy_attribute(object_at(p,0xb90),factories[i],desired);
  }
}

void OnRep_ReplayRepAnimMontageInfo(Pawn* p) {
  copy_bytes(p,0x2020,p,0x2990,0x38);
  Object* ability=object_at(p,0xbb8);
  if (ability && is_a(ability,0x14277e480)) {
    assign_montage_1406daaf0(ability,p,0x2020);
    copy_bytes(ability,0x14a8,p,0x2060,0x28);
    vcall_7d8(object_at(p,0xbb8));
  }
}

void OnRep_MeshHeightAdjustAthena(Pawn* p) {
  Object* mesh=object_at(p,0x278);
  if (!mesh) return;
  Vec3 location{0,0,float_at(p,0x2968)}; // MeshHeightAdjustAthena, verify offset in reflection before integration
  Rotation rotation=load_vec3(mesh,0x128);
  Rotation cached=load_vec3(mesh,0x1b0);
  if (rotation.x!=cached.x || rotation.y!=cached.y || rotation.z!=cached.z) {
    Rotation normalized=normalize_rotation_native(rotation);
    store_vec3(mesh,0x1b0,normalized);
    Quat q=rotation_to_quat(normalized);
    std::memcpy(ptr_at(mesh,0x1a0),&q,sizeof(q));
  }
  Quat q; std::memcpy(&q,ptr_at(mesh,0x1a0),sizeof(q));
  mesh_set_relative_location_rotation(mesh,location,q);
  pawn_vcall_760(p,load_vec3(mesh,0x11c),load_vec3(mesh,0x128));
}

void SetFallInstigator(Pawn* p, Object* instigator, const TagContainer& tags) {
  TagContainer temporary{};
  assign_tags(temporary,tags);
  object_at(p,0x2df0)=instigator;
  assign_tags(*static_cast<TagContainer*>(ptr_at(p,0x2df8)),temporary);
  Object* w=world(p);
  Object* controller=object_at(p,0x250);
  if (w && controller && is_a(controller,0x142a3d9f0)) {
    Object* typed=object_at(p,0x2df0);
    if (typed && !is_a(typed,0x142a3d9f0)) typed=nullptr;
    object_at(controller,0x808)=typed;
    float_at(controller,0x810)=float_at(w,0x540); // bitwise 32-bit copy
  }
  destroy_tags(temporary);
}

void GamePhaseStepChanged(Pawn* p, Object* safeZoneObject, uint8_t step) {
  if (!safeZoneObject || !is_a(safeZoneObject,0x0 /* AFortGameStateAthena_StaticClass */)) return;
  bool active=static_cast<uint8_t>(step-5)<6;
  if ((byte_at(p,0x2f90)!=0)!=active) byte_at(p,0x2f90)=active;
  if (step==11 && ((byte_at(p,0x2f75)!=0 && active) || global_148114c00())) call_141a0b170(p);
}
