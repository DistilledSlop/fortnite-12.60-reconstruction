// Uncounted rough control-flow lifts. These adapters and layouts are unresolved;
// this file is neither an executable implementation nor a verified ABI.
#include <cstdint>
#include <cmath>
struct Obj; struct Pawn; struct Vec { float x,y,z; }; struct Guid { uint32_t a,b,c,d; }; struct Transform { uint8_t opaque[0x88]; }; struct Cue; struct Damage; struct Text;
extern uint8_t& b(void*,unsigned); extern int& i(void*,unsigned); extern float& f(void*,unsigned); extern Obj*& o(void*,unsigned); extern void* at(void*,unsigned);
extern bool is_object_class(Obj*,uintptr_t); extern bool is_class_child(Obj*,uintptr_t); extern Obj* call_142321a70(Pawn*); extern Obj* class_at(uintptr_t); extern Obj* camera_create(Obj*,unsigned,Obj*,uint8_t); extern float pawn_a00(Pawn*); extern void camera_view(Obj*,void*,void*); extern uint32_t camera_6c0(Obj*); extern void effect_660(Obj*,void*,void*,uint32_t); extern void call_141ca4780_then_1420fadd0(Obj*);
extern void pooled_bind_completed_weak_delegate(Obj*,Pawn*); // resolve name, weak target, FindFunction, append delegate including grow
extern Vec transform_location_plus_forward(Transform*,float); extern void set_effect_vec(Obj*,unsigned,Vec); extern void copy_camera_460_to_effect_2b0(Obj*,Obj*);
Obj* SpawnLens(Pawn* p,Obj* cls,Obj* source,Transform* transform,uint8_t flag,bool pooled) {
 Obj* controller=call_142321a70(p); Obj* camera=nullptr;
 if (controller && is_object_class(controller,0x142a3d9f0)) {
  camera=o(controller,0x2b0); if (camera && !is_object_class(camera,0x142a35cb0)) camera=nullptr;
 } else controller=nullptr;
 if (!camera || !cls || !class_at(0x1428dd620) || !is_class_child(cls,0x1428dd620)) return nullptr;
 Obj* checked=(class_at(0x1428dd620) && is_class_child(cls,0x1428dd620))?cls:nullptr;
 Obj* effect=camera_create(camera,pooled?0x838:0x830,checked,flag);
 if (!effect || !is_object_class(effect,0x1428dd620)) return nullptr;
 call_141ca4780_then_1420fadd0(controller);
 if (pooled) { if (o(effect,0x218)) pooled_bind_completed_weak_delegate(o(effect,0x218),p); b(effect,0x220)&=0xfe; }
 if (!source) { if (pooled) o(effect,0x2d0)=nullptr; set_effect_vec(effect,0x2d8,transform_location_plus_forward(transform,pawn_a00(p))); }
 else o(effect,0x2d0)=source;
 copy_camera_460_to_effect_2b0(camera,effect);
 alignas(16) uint8_t a[16],c[16]; camera_view(camera,a,c); uint32_t mode=camera_6c0(camera); effect_660(effect,a,c,mode); return effect;
}
extern Obj* anim_instance_if_mesh_present(Pawn*); extern float montage_play_143fd6e10(Obj*,Obj*,float,int,int,int); extern bool fname_is_none(uint64_t); extern void montage_section_143fd6b30(Obj*,uint64_t,Obj*); extern Obj* montage_instance_143fca900(Obj*,Obj*); extern void call_143fa3a20(void*,float);
float PlayLocalAnimMontage(Pawn* p,Obj* montage,float rate,uint64_t section,bool adjust) {
 Obj* anim=anim_instance_if_mesh_present(p); if (!montage || !anim) return 0;
 float result=montage_play_143fd6e10(anim,montage,rate,0,0,1); if (result<=0) return 0;
 if (!fname_is_none(section)) montage_section_143fd6b30(anim,section,montage);
 if (adjust) { Obj* instance=montage_instance_143fca900(anim,montage); if(instance) call_143fa3a20(at(instance,0x130),1.0f); } return result;
}
extern void sentence_text_141d50140(void* sentence,Text* result);
Text* GetCurrentSentenceSpeechText(Pawn* p,Text* result) { sentence_text_141d50140(at(p,0xd90),result); return result; }
extern void hit_angle_142c521d0(float out[2],float last,const Vec*,const Vec*,const Vec*); // incoming direction/param2 ABI remains unresolved
void SetFNameBasedOnHitDirection(Pawn* p,void* direction,uint64_t* out,uint64_t front,uint64_t back,uint64_t left,uint64_t right) {
 Obj* root=o(p,0x130); float x=0,y=0,z=0,w=1; if(root){x=f(root,0x1c0);y=f(root,0x1c4);z=f(root,0x1c8);w=f(root,0x1cc);}
 float z2=z*2.0f, y2=y*2.0f, x2sq=x*x*2.0f;
 float wx2=w*x*2.0f;
 Vec a{1.0f-(z*z2+y*y2),w*z2+x*y2,x*z2-w*y2};
 Vec c{x*y2-w*z2,1.0f-(z*z2+x2sq),wx2+y*z2};
 Vec d{w*y2+x*z2,y*z2-wx2,1.0f-(y*y2+x2sq)};
 // The decompiler omits param2/direction from this call; its true ABI is unresolved.
 (void)direction; float angles[2]; hit_angle_142c521d0(angles,c.z,&a,&c,&d);
 float angle=angles[0]*57.295776f; float abs=std::fabs(angle);
 if (45.0f < abs) { if (abs < 120.0f) { back=left; if (0.0f < angle) back=right; } *out=back; }
 else *out=front;
}
// Equip adapters expose native operations, including reference ownership and failure logs.
extern bool guid_equal(Guid,Guid); extern Guid weapon_guid(Obj*); extern bool weapon_live_flags(Obj*); extern void equip_existing_142314fe0(Pawn*,Obj*);
extern void call_141ed2350(); extern bool call_141ed8de0(Pawn*,int); extern uint8_t definition_298(Obj*); extern Obj* harvesting_override_weapon(Obj* controller); extern Obj* harvesting_definition_1423800b0(Obj*); extern void definition_get_class_142672c10(Obj*,void* out); extern Obj* definition_class(Obj*); extern Obj* spawn_weapon_1442e6580(Pawn*,Obj*); extern void weapon_initialize_1426b34e0(Obj*,Guid,Obj*); extern void copy_harvest_data(Obj*,Obj*); extern void log_missing_harvest_definition(); extern void weapon_finish_spawning_144211f60(Obj*); extern void weapon_list_remove_then_prepend(Pawn*,Obj*); extern void weapon_760(Obj*,Pawn*); extern bool pawn_c50(Pawn*); extern void weapon_850(Obj*); extern void log_spawn_failure_class(Obj*);
void EquipWeaponDefinition(Pawn* p,Obj* definition,Guid guid) {
 if (b(p,0xf0)!=3) return;
 for(int n=0;n<i(p,0x5a0);++n) { Obj* w=reinterpret_cast<Obj**>(o(p,0x598))[n]; if(w && guid_equal(weapon_guid(w),guid)) { if(weapon_live_flags(w)){equip_existing_142314fe0(p,w);return;} break; } }
 if(!definition) return;
 Obj* alternate=nullptr; Obj* harvest=nullptr; call_141ed2350();
 bool harvesting=call_141ed8de0(p,0) && definition_298(definition)==0x17;
 if(harvesting) { Obj* controller=o(p,0x250); if(controller && is_object_class(controller,0x142a3d9f0)) { alternate=harvesting_override_weapon(controller); if(alternate){ definition_get_class_142672c10(alternate,&guid); definition=alternate; } harvest=harvesting_definition_1423800b0(controller); } }
 Obj* cls=definition_class(definition); if(!cls || !class_at(0x142b43730) || !is_class_child(cls,0x142b43730)) return;
 // Spawn helper must reproduce native world lookup, root location/global fallback, transform,
 // spawn parameter construction (owner/instigator pawn, flag 4), and second class check.
 Obj* weapon=spawn_weapon_1442e6580(p,cls);
 if(weapon && is_object_class(weapon,0x142b43730)) {
  o(weapon,0x358)=alternate; weapon_initialize_1426b34e0(weapon,guid,definition);
  Obj* data=o(weapon,0x350); if(harvesting && data) { if(!harvest) log_missing_harvest_definition(); else copy_harvest_data(data,harvest); }
  weapon_finish_spawning_144211f60(weapon); weapon_list_remove_then_prepend(p,weapon); weapon_760(weapon,p); equip_existing_142314fe0(p,weapon); if(pawn_c50(p)) weapon_850(weapon); return;
 }
 log_spawn_failure_class(cls); // conditional verbosity, object-name conversion and freeing remain unresolved
}
extern bool sound_effect_enabled_14230c690(Pawn*); extern bool sound_tracking_global(); extern bool sound_visibility_global(); extern bool sound_visible_141ef7100(Pawn*,Obj*); extern void update_sound_tracking_map(Pawn*,Obj*,Obj*,Vec,float); extern void find_sound_effect_142345160(Pawn*,Obj*,uint8_t,Obj**,Obj**); extern int runtime_sound_limit(); extern bool runtime_sound_pooled(); extern float sound_priority_142324820(Pawn*,float,uint8_t); extern void sound_reset_141f69280(Obj*); extern void sound_intensity_141e80780(Obj*,float); extern void sound_configure_141f62920(Obj*,Obj*,uint8_t,float,const Guid&,uint64_t); extern Transform sound_transform(Vec); extern void insert_sound_sorted(Pawn*,int,Obj*); extern Obj* sound_list_at(Pawn*,int); extern void sound_spawn_failure_log(Pawn*,Obj*); extern void destroy_143f6c200(Obj*,int,int); extern Vec root_location_or_global(Pawn*);
Obj* SpawnSoundLensEffectDirectional(Pawn* p,Obj* source,Vec position,float range,uint8_t kind,Obj* instigator,const Guid& id,uint64_t arg8) {
 Obj* cls=o(p,0xa80); bool visual=sound_effect_enabled_14230c690(p) && cls && class_at(0x1428dd620) && is_class_child(cls,0x1428dd620);
 bool track=sound_tracking_global() && kind==2; if(!visual && !track) return nullptr;
 Vec pos=root_location_or_global(p); float dx=pos.x-position.x,dy=pos.y-position.y,dz=pos.z-position.z; float ratio=std::sqrt(dx*dx+dy*dy+dz*dz)/range;
 float clamped=0; if(0<=ratio) { clamped=ratio; if(1<=ratio) clamped=1; } float intensity=1-clamped; if(intensity<=0.01f) return nullptr;
 if(track && instigator && is_object_class(instigator,0x142a174f0) && (sound_visibility_global() || sound_visible_141ef7100(p,instigator))) update_sound_tracking_map(p,instigator,source,position,intensity);
 if(!visual) return nullptr;
 Obj *existing=nullptr,*lowest=nullptr; find_sound_effect_142345160(p,source,kind,&existing,&lowest);
 if(existing){sound_reset_141f69280(existing);sound_intensity_141e80780(existing,intensity);return existing;}
 int limit=runtime_sound_limit(); if(limit>0 && limit<=i(p,0xa78)) {
  if(sound_priority_142324820(p,intensity,kind)<=sound_priority_142324820(p,f(lowest,0x2f0),b(lowest,0x2f8))) return nullptr;
  sound_configure_141f62920(lowest,instigator,kind,range,id,arg8); if(lowest){sound_reset_141f69280(lowest);sound_intensity_141e80780(lowest,intensity);return lowest;}
 }
 Transform t=sound_transform(position); cls=o(p,0xa80); if(!cls || !class_at(0x1428dd620) || !is_class_child(cls,0x1428dd620)) cls=nullptr;
 Obj* effect=SpawnLens(p,cls,source,&t,0,runtime_sound_pooled());
 if(effect && is_object_class(effect,0x142adb4c0)) {
  float priority=sound_priority_142324820(p,intensity,kind); int n=i(p,0xa78)-1;
  for(;n>=0;--n){Obj* e=sound_list_at(p,n);if(e && is_object_class(e,0x142adb4c0) && priority<sound_priority_142324820(p,f(e,0x2f0),b(e,0x2f8))) break;}
  insert_sound_sorted(p,n+1,effect); sound_configure_141f62920(effect,instigator,kind,range,id,arg8); sound_intensity_141e80780(effect,intensity); return effect;
 }
 sound_spawn_failure_log(p,effect); if(effect) destroy_143f6c200(effect,0,1); return nullptr;
}
// Damage cue adapters retain object ownership/callback behavior as unresolved contracts.
extern void ensure_cue_shared_owner(Pawn*); // acquire 1406c6ef0/v270, allocate shared control, swap and release old references
extern Obj* cue_resolve_1415f4460(Pawn*); extern void cue_initialize_if_empty(Obj*); extern void cue_10(Obj*,Obj*,Pawn*); extern Obj* cue_weapon_context(Obj*); extern void cue_set_context_14160d0f0(Obj*,Obj*,float); extern void weak_set_142ee3b30(void*,Obj*); extern void fill_hit_data(Obj*,Pawn*,Damage*,Obj*); extern bool pawn_6b0(Pawn*); extern void weapon_a48(Obj*); extern bool weapon_9b8(Obj*); extern Obj* pawn_150(Pawn*); extern bool timer_active_unless_state_two(Pawn*,Obj*); extern void apply_damage_tag_1406ce540(Pawn*,uintptr_t); extern void schedule_damage_timer_1446278d0(Pawn*,Obj*,float); // weak delegate 142353440; clean delegate storage after registration
extern void pawn_cue_8a8_copy(Pawn*,uintptr_t,Cue*); extern Obj* find_component_14070ae90(Obj*,int); extern bool component_ready_142ecc7b0(Obj*,int); extern Obj* component_590(Obj*); extern void component_cue_80_copy(Obj*,uintptr_t,Cue*); extern void append_cue_copy(Obj*,Cue*);
void NetMulticast_Athena_BatchedDamageCues_Helper(Pawn* p,Damage* damage,Obj** instigator) {
 if(!o(p,0xef0)) ensure_cue_shared_owner(p);
 Obj* context=cue_resolve_1415f4460(p); if(!context) return;
 cue_initialize_if_empty(context); cue_10(context,o(p,0x238),p);
 Obj* weapon=o(p,0x588); cue_set_context_14160d0f0(context,weapon?cue_weapon_context(weapon):nullptr,1.0f);
 b(context,0x70)=b(damage,0x1d); b(context,0x71)=b(damage,0x1e);
 weak_set_142ee3b30(at(context,0x10),weapon); weak_set_142ee3b30(at(p,0xf70),weapon);
 f(p,0xee4)=f(damage,0x18); weak_set_142ee3b30(at(p,0xf78),p);
 Obj* target=*instigator; fill_hit_data(context,p,damage,target); // writes repeated impact position, normal, weapon location and weak target
 Cue* cue=reinterpret_cast<Cue*>(at(p,0xee0));
 if(b(damage,0x21)) pawn_cue_8a8_copy(p,0x298,cue);
 if(!pawn_6b0(p)) {
  if(weapon) weapon_a48(weapon);
  if(!b(damage,0x1c)) {
   if(!(b(p,0x522)&0x80) && o(p,0xbb8) && weapon && weapon_9b8(weapon)) {
    Obj* world=pawn_150(p); if(world && !timer_active_unless_state_two(p,world)) { apply_damage_tag_1406ce540(p,0x1518); schedule_damage_timer_1446278d0(p,world,0.2f); }
   }
  } else pawn_cue_8a8_copy(p,0x1518,cue);
  pawn_cue_8a8_copy(p,0x14f0,cue);
 }
 float amount=f(p,0xee4); if(amount<=0.0f && amount!=0.0f) return;
 Obj* typed=target && is_object_class(target,0x142732ac0)?target:nullptr;
 Obj* component=find_component_14070ae90(target,0);
 if(!component) { if(typed) append_cue_copy(typed,cue); }
 else if(component_ready_142ecc7b0(component,1)) {
  Obj* receiver=component_590(component); if(receiver) {
   uintptr_t tag=b(damage,0x1f)?0x248:(b(damage,0x22)?0x1510:0x14f8);
   component_cue_80_copy(receiver,tag,cue); if(b(damage,0x20)) component_cue_80_copy(receiver,0x280,cue);
  }
 }
}
