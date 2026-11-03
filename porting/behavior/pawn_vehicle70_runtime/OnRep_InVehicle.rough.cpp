// Uncounted rough reconstruction of OnRep_InVehicle, source lines 103-1177.
// Neutral adapters denote individual operations/fields; no ABI or parity claim.
#include <cstdint>
struct Pawn; struct Object; struct Vehicle; struct Name; struct OwnedString;
struct VehicleState { unsigned char bytes[32]; };
extern int vehicle_log_verbosity();
extern Vehicle* pawn_v1088(Pawn*); extern bool vehicle_v568(Vehicle*, uint8_t);
extern uint8_t role(Pawn*); extern void* fun_142d68a40();
extern Object* find_enum_142ed84a0(void*, int, const wchar_t*, int);
extern void log_enum_missing(); extern void fun_142c7b670();
extern OwnedString* enum_name_142d67ac0(Object*, uint8_t);
extern void log_role_driving_unresolved_args(OwnedString*); // decompiler omits variadic arguments
extern void free_string(OwnedString*);
extern Object* movement(Pawn*); extern void log_no_movement(); extern void pawn_v5e8(Pawn*);
extern VehicleState& local_state(Pawn*); extern VehicleState& rep_state(Pawn*);
extern Object* state_vehicle(const VehicleState&); extern uint8_t state_seat(const VehicleState&);
extern uint8_t state_byte_e(const VehicleState&); extern Vehicle* cast_vehicle_142ec9900(Object*);
extern void initialize_name_once_142cd4ab0(const wchar_t*, Name&);
extern Name& holster_driving(); extern Name& holster_passenger();
extern bool global_148157d24(); extern uint8_t& held_object_byte8(Pawn*);
extern uint8_t& movement_byte_16a(Object*); extern Object* controller(Pawn*);
extern Object* controller_if_class_142a62c80(Object*); extern bool controlled_rc_valid(Pawn*);
extern Object* controlled_rc_controller(Pawn*); extern void fun_1423d7520(Object*);
extern Object* mesh_270(Pawn*); extern Object*& pre_driving_anim_bp(Pawn*);
extern bool class_is_1446b91c0(Object*); extern void fun_14234b2e0(Pawn*);
extern Object* current_vehicle_anim_layer(Pawn*); extern void fun_14234b3f0(Pawn*,Object*,Object*);
extern Object* fun_1419d2b60(Object*); extern uint8_t& object_byte_16(Object*);
extern void vehicle_vd8(Vehicle*,uint8_t,bool); extern void vehicle_v1d0(Vehicle*,Pawn*);
extern void vehicle_v78(Vehicle*,Pawn*,uint8_t); extern void vehicle_v140(Vehicle*,Pawn*);

// This state survives across subsequent sections: the complete continuation must
// preserve the original snapshots even if callbacks mutate the pawn's states.
struct InVehicleFrame {
 Object* movement; Object* oldVehicle; Object* newVehicle; Object* exitMesh;
 Vehicle* oldInterface; Vehicle* newInterface;
 uint32_t oldSeat; uint8_t oldByteE, newByteE;
 bool leaving, entering, seatChanged, newSeatDriving, bothNull;
};

// Returns false on the native invalid-movement early return; caller continues at line 257 only if true.
bool OnRep_InVehicle_prefix(Pawn* p, InVehicleFrame& f) {
 if (vehicle_log_verbosity() > 4) {
  Vehicle* current = pawn_v1088(p);
  if (current) vehicle_v568(current, state_seat(local_state(p)));
  uint8_t r = role(p);
  Object* e = find_enum_142ed84a0(fun_142d68a40(), 0, L"Engine.ENetRole", 0);
  if (!e) { log_enum_missing(); fun_142c7b670(); }
  OwnedString* name = enum_name_142d67ac0(e, r);
  log_role_driving_unresolved_args(name);
  if (name) free_string(name);
 }
 f.movement = movement(p);
 if (!f.movement) { if (vehicle_log_verbosity() > 1) log_no_movement(); return false; }
 if (role(p) == 3) pawn_v5e8(p);
 f.oldVehicle = state_vehicle(local_state(p)); f.newVehicle = state_vehicle(rep_state(p));
 f.leaving = f.oldVehicle != f.newVehicle && f.oldVehicle;
 f.entering = f.oldVehicle != f.newVehicle && f.newVehicle;
 f.seatChanged = f.oldVehicle == f.newVehicle && f.oldVehicle &&
                 state_seat(local_state(p)) != state_seat(rep_state(p));
 f.oldByteE = state_byte_e(local_state(p)); f.newByteE = state_byte_e(rep_state(p));
 f.oldInterface = f.oldVehicle ? cast_vehicle_142ec9900(f.oldVehicle) : nullptr;
 Object* replicated = state_vehicle(rep_state(p));
 f.newInterface = replicated ? cast_vehicle_142ec9900(replicated) : nullptr;
 f.oldSeat = state_vehicle(local_state(p)) ? state_seat(local_state(p)) : UINT32_MAX;
 // Native assumes the cast succeeded when a seat changes.
 f.newSeatDriving = f.seatChanged && vehicle_v568(f.oldInterface, state_seat(rep_state(p)));
 initialize_name_once_142cd4ab0(L"HolsterDriving", holster_driving());
 initialize_name_once_142cd4ab0(L"HolsterPassenger", holster_passenger());
 f.bothNull = f.oldVehicle == f.newVehicle && !state_vehicle(local_state(p));
 if (f.leaving || f.bothNull) {
  if (global_148157d24() && held_object_byte8(p)) {
   movement_byte_16a(f.movement) = held_object_byte8(p); held_object_byte8(p) = 0;
  }
  Object* c = controller_if_class_142a62c80(controller(p));
  if (!c && controlled_rc_valid(p)) c = controller_if_class_142a62c80(controlled_rc_controller(p));
  if (c) fun_1423d7520(c);
  Object* mesh = mesh_270(p); f.exitMesh = mesh;
  Object* anim = pre_driving_anim_bp(p);
  if (anim && class_is_1446b91c0(anim) && mesh) {
   fun_14234b2e0(p); pre_driving_anim_bp(p) = nullptr;
  }
  fun_14234b3f0(p, current_vehicle_anim_layer(p), nullptr);
  object_byte_16(fun_1419d2b60(controller(p))) |= 1;
  if (f.oldInterface) {
   vehicle_vd8(f.oldInterface,state_seat(local_state(p)),false);
   vehicle_v1d0(f.oldInterface,p);
   vehicle_v78(f.oldInterface,p,state_seat(local_state(p)));
   vehicle_v140(f.oldInterface,p);
  }
  // Continuation starts within this branch at line 257.
 }
 return true;
}

// Continuation, source 257–551. These adapters each represent the indicated
// native operation or field; the two quaternion conversion helpers do not own
// their input or output. Symbols and offsets remain unresolved.
struct Vec3 { float x,y,z; }; struct Vec4 { float x,y,z,w; };
extern Object* component_128(Pawn*); extern Object* component_280(Pawn*);
extern Vec4& component_rotation_180(Object*); extern Vec4 component_rotation_1c0(Object*);
extern Vec3& component_rotator_190(Object*); extern Vec3 component_vector_1d0(Object*);
extern Vec3 quat_to_rotator_142c5d000(Vec4); extern Vec3 direction_142c620f0(Vec3);
extern Vec3 global_148218630(); extern Vec3 global_148218640(); extern Vec3 global_147c31cb0();
extern void fun_143f6cf60(Pawn*,const void*); extern const void* dat_147cdd8e4();
extern void fun_143f83ff0(Pawn*,Vec3,bool); extern Vec3 pawn_v778(Pawn*);
extern Vec3 mesh_vector_11c(Object*); extern void fun_144122860(Object*,Vec3,const void*,bool);
extern const void* dat_147c31d50(); extern uint8_t& component_byte_14d(Object*);
extern float& fun_14070f0d0_float20(Pawn*); extern void pawn_v420(Pawn*);
extern Object* controller_if_142a3d9f0(Object*); extern Vec3 controller_v660(Object*);
extern void controller_v668(Object*,Vec3); extern void controller_v6f8(Object*,Vec3&,Vec3&);
extern uint8_t& movement_byte_1f0(Object*); extern uint8_t& movement_byte_38c(Object*);
extern uint32_t& pawn_flags_218(Pawn*); extern uint32_t blendables_flags_c(Pawn*);
extern void fun_144124090(Object*,bool); extern float& vehicle_input_float8(Pawn*);
extern Object* pawn_v150(Pawn*); extern float object_float_540(Object*); extern float dat_1478de524();
extern void component_v800(Object*,int,int); extern void component_v800_enter(Object*,int);
extern Vec3 actor_v2f0(Object*); extern bool fun_142331180(Pawn*);
extern float vehicle_v208(Vehicle*); extern float vehicle_v210(Vehicle*);
extern float vehicle_v218(Vehicle*); extern float vehicle_v220(Vehicle*);
extern bool vehicle_v1e8(Vehicle*); extern void movement_v638(Object*,Vec3);
extern Object* movement_if_1429f5d30(Object*); extern uint32_t state_word8(const VehicleState&);
extern void fun_1420edbb0(Object*,uint32_t); extern bool holster_contains(Pawn*,Name&);
extern Name& holster_148158d28(); extern void pawn_vc60(Pawn*,bool,int);
extern void vehicle_v50(Vehicle*,void*); extern void* crouch_mesh_offset4(Pawn*);
extern void fun_142356640(Pawn*,int); extern void fun_142a769a0(Pawn*);
extern void fun_142349360(Pawn*); extern Object* current_weapon(Pawn*);
extern void fun_1426abce0(Object*,Object*); extern void vehicle_v1a8_exit(Vehicle*,Pawn*);
extern Object* vehicle_v3b8(Vehicle*,Pawn*); extern void fun_1423ec450(Object*,uint8_t);
extern Object* vehicle_v3b0(Vehicle*,Pawn*); extern void fun_142a8ae70();
extern void pawn_v1388(Pawn*); extern void controller_v1f78(Object*,Object*,int);
extern void vehicle_v1e0(Vehicle*); extern void component_v618(Object*,bool);
extern void vehicle_v70(Vehicle*,uint8_t,bool); extern bool vehicle_vc8(Vehicle*,uint8_t);
extern Vec3 vehicle_ve0(Vehicle*,uint8_t); extern uintptr_t vehicle_v90(Vehicle*);
extern void pawn_v758(Pawn*,uintptr_t,Vec3,bool);
extern void fun_143f69330(Pawn*,Object*,const void*,Vec3); extern const void* dat_147cdd8d8();
extern void pawn_v4f0(Pawn*,Vec3,Vec3,bool); extern Vec3 vehicle_va0(Vehicle*,uint8_t,Pawn*);
extern void copy_vehicle_state_32(VehicleState&,const VehicleState&); // eight 32-bit stores, preserving native read/store order
extern Vec3 normalize_native(Vec3); // squared length == 1 unchanged, below 1e-8 global_148218630; otherwise two rsqrt refinements

void OnRep_InVehicle_257_551(Pawn* p, InVehicleFrame& f) {
 Vehicle* old = f.oldInterface; Vehicle* next = f.newInterface;
 Object* m = f.movement;
 if (f.leaving || f.bothNull) {
  fun_143f6cf60(p,dat_147cdd8e4());
  Object* component = component_128(p);
  float y;
  if (!component) y = global_148218640().y;
  else {
   Vec4 current = component_rotation_1c0(component);
   Vec4& prior = component_rotation_180(component);
   if (prior.x != current.x || prior.y != current.y || prior.z != current.z || prior.w != current.w) {
    prior = current; component_rotator_190(component) = quat_to_rotator_142c5d000(current);
   }
   y = component_rotator_190(component).y;
  }
  Vec3 exitRotation{0,y,0};
  fun_143f83ff0(p,exitRotation,false);
  if (f.exitMesh) {
   Vec3 pawnTransform = pawn_v778(p);
   Vec3 meshTransform = mesh_vector_11c(f.exitMesh);
   fun_144122860(f.exitMesh,meshTransform,&pawnTransform,false);
  }
  component_byte_14d(component_128(p)) &= 0xef;
  if (role(p)==1) { fun_14070f0d0_float20(p)+=0.5f; pawn_v420(p); }
  Object* c = controller_if_142a3d9f0(controller(p));
  if (c) {
   Vec3 r = exitRotation;
   if (f.oldByteE == f.newByteE) { r=controller_v660(c); r.z=0; }
   controller_v668(c,r);
  }
  movement_byte_1f0(m) |= 4;
  fun_144124090(component_280(p),false); movement_byte_38c(m) &= 0xbf;
  pawn_flags_218(p) = (pawn_flags_218(p)&~2u) | ((blendables_flags_c(p)&1u)*2u);
  if (vehicle_input_float8(p)==0) {
   Object* o=pawn_v150(p);
   vehicle_input_float8(p)=o ? dat_1478de524()+object_float_540(o) : 0.0f;
  }
  component_v800(component_280(p),6,2);
  if (!f.entering && old && f.oldByteE==f.newByteE) {
   Vec3 forward=actor_v2f0(state_vehicle(local_state(p)));
   bool applyAux=false;
   if (!fun_142331180(p)) {
    Object* c2=controller_if_142a3d9f0(controller(p));
    if (c2) {
     float speed=vehicle_v218(old), vertical=vehicle_v220(old);
     Vec3 a, angles; controller_v6f8(c2,a,angles); angles.x=0; angles.z=0;
     Vec3 direction=direction_142c620f0(angles), n=normalize_native(forward);
     n=normalize_native({n.x+direction.x,n.y+direction.y,n.z+direction.z});
     movement_v638(m,{n.x*speed,n.y*speed,n.z*speed+vertical});
     applyAux=vehicle_v1e8(old);
    }
   } else {
    float speed=vehicle_v208(old), vertical=vehicle_v210(old);
    movement_v638(m,{forward.x*speed,forward.y*speed,forward.z*speed+vertical});
    applyAux=true;
   }
   if (applyAux) fun_1420edbb0(movement_if_1429f5d30(m),state_word8(rep_state(p)));
  }
  // Each check iterates the live 8-byte FName array separately; the first
  // callback can mutate it before the second scan.
  if (holster_contains(p,holster_driving())) pawn_vc60(p,false,0);
  if (holster_contains(p,holster_148158d28())) pawn_vc60(p,false,0);
  if (state_vehicle(local_state(p))) vehicle_v50(old,crouch_mesh_offset4(p));
  fun_142356640(p,5); copy_vehicle_state_32(local_state(p),rep_state(p));
  fun_142a769a0(p); fun_142349360(p);
  Object* w=current_weapon(p);
  if (w && !state_vehicle(local_state(p))) fun_1426abce0(w,w);
  if ((f.leaving || f.bothNull) && old && f.oldSeat!=UINT32_MAX) vehicle_v1a8_exit(old,p);
 }
 if (f.entering) {
  Object* o=vehicle_v3b8(next,p);
  if (o) fun_1423ec450(o,state_seat(rep_state(p)));
  if (role(p)==3) {
   Object* v=state_vehicle(rep_state(p));
   if (v) { Vehicle* vi=cast_vehicle_142ec9900(v); if (vi && vehicle_v3b0(vi,p)) fun_142a8ae70(); }
  }
  copy_vehicle_state_32(local_state(p),rep_state(p)); pawn_v1388(p);
  Object* c=controller_if_class_142a62c80(controller(p));
  if (c) controller_v1f78(c,state_vehicle(local_state(p)),0);
  vehicle_v1e0(next); component_v618(component_280(p),true);
  vehicle_vd8(next,state_seat(local_state(p)),true);
  vehicle_v70(next,state_seat(local_state(p)),false);
  if (vehicle_vc8(next,state_seat(local_state(p)))) {
   fun_144124090(component_280(p),true); movement_byte_38c(m)|=0x40;
  }
  Vec3 seat=vehicle_ve0(next,state_seat(local_state(p)));
  uintptr_t attachment=vehicle_v90(next);
  pawn_v758(p,attachment,seat,true);
  fun_143f69330(p,state_vehicle(local_state(p)),dat_147cdd8d8(),seat);
  Vec3 rotation=global_148218640();
  Object* c2=controller(p); if (c2) { Vec3 unused; controller_v6f8(c2,unused,rotation); }
  Object* component=component_128(p);
  Vec3 position=component ? component_vector_1d0(component) : global_148218630();
  Vec3 delta=global_147c31cb0(); position={position.x+delta.x,position.y+delta.y,position.z+delta.z};
  pawn_v4f0(p,position,rotation,false);
  Vec3 vehicleTransform=vehicle_va0(next,state_seat(local_state(p)),p);
  fun_144122860(component_128(p),vehicleTransform,dat_147c31d50(),false);
  component_v800_enter(component_280(p),6);
  // Continuation stays inside this branch at source line 552.
 }
}

// Source 552–856. The returned cursor distinguishes the native jumps around
// the controller-input branch, including the early function exit at line 789.
struct SharedRef; struct EventTemp;
extern void movement_v460(Object*); extern void movement_vb00(Object*); extern Vec3& movement_vector_c4(Object*);
extern void pawn_v798(Pawn*); extern void* vehicle_v48(Vehicle*,uint8_t);
extern void*& vehicle_input(Pawn*); extern void controller_vbd8(Object*,void*);
extern void*& controller_input_7a0(Object*); extern bool vehicle_v560(Vehicle*,uint8_t);
extern void pawn_vb68(Pawn*,int); extern void pawn_vab0(Pawn*,int); extern void fun_1423a8b00(Object*,int);
extern Object* mesh_anim_class_628(Object*); extern Object* vehicle_v2a0(Vehicle*,uint8_t);
extern bool vehicle_v178(Vehicle*); extern Object* fun_144114bd0(Object*); extern void fun_143fe17f0(Object*,int);
extern void fun_14234b2e0_with_class(Pawn*,Object*); extern Object* vehicle_v158(Vehicle*);
extern Object* vehicle_v160(Vehicle*); extern bool fun_142331940(Pawn*); extern bool fun_14230c8b0(Pawn*);
extern uint8_t& footstep_surface(Pawn*); extern Object* event_target_v10(Pawn*);
extern EventTemp* event_temp_construct(Pawn*); // zero native members, vtable DAT_1459aede0, inline array metadata
extern void event_set_surface_141e60520_143e8b3b0(EventTemp*,uint8_t);
extern void event_target_v620(Object*,EventTemp*); // uses DAT from FUN_141ed1800()+0x1f0
extern int event_ref_count(EventTemp*); extern SharedRef* event_ref_at(EventTemp*,int); // inline or heap array
extern int& shared_strong(SharedRef*); extern int& shared_weak(SharedRef*);
extern void shared_destroy_v0(SharedRef*); extern void shared_free_v8(SharedRef*,int);
extern void* event_heap_refs(EventTemp*); extern void* event_heap_d8(EventTemp*);
extern void* event_heap_e8(EventTemp*); extern void* event_heap_f8(EventTemp*); extern void* event_heap_108(EventTemp*);
extern SharedRef* event_shared_110(EventTemp*); extern void event_restore_vtable(EventTemp*);
extern void memory_free(void*); extern void event_temp_end_stack(EventTemp*); // end stack lifetime; no owned cleanup
extern void fun_142a768d0(Pawn*);
extern Object* vehicle_actor_component_130(Object*); extern bool object_class_is_1447c8590(Object*);
extern Vec3 vehicle_v98_translation(Vehicle*,uint8_t,int); extern void state_set_vector_10(VehicleState&,Vec3);
extern void fun_1420d8580(Object*); extern void fun_1420d8560();

static void release_event_ref(SharedRef* ref) {
 if (!ref) return;
 if (--shared_strong(ref)==0) {
  shared_destroy_v0(ref);
  if (--shared_weak(ref)==0) shared_free_v8(ref,1);
 }
}

enum class InVehicleNext { DoneEarly, SeatControllerInput, AfterSeatControllerInput, AfterSeatChange };
struct InVehicle552Frame { Object* seatController; };
static void old_driver_seat_transition(Pawn* p, InVehicleFrame& f) {
 Vehicle* old=f.oldInterface; Vehicle* next=f.newInterface;
 if (vehicle_v568(old,state_seat(local_state(p)))) {
  object_byte_16(fun_1419d2b60(controller(p))) |= 1;
  if (mesh_270(p)) {
   Object* anim=vehicle_v2a0(next,state_seat(rep_state(p)));
   if ((anim && class_is_1446b91c0(anim)) ||
       (pre_driving_anim_bp(p) && class_is_1446b91c0(pre_driving_anim_bp(p)))) fun_14234b2e0(p);
   fun_14234b3f0(p,current_vehicle_anim_layer(p),vehicle_v160(next));
  }
  if (holster_contains(p,holster_driving())) pawn_vc60(p,false,0);
  Object* c2=controller_if_142a3d9f0(controller(p));
  if (c2) { Vec3 r=controller_v660(c2); r.z=0; controller_v668(c2,r); }
 }
}
InVehicleNext OnRep_InVehicle_552_856(Pawn* p, InVehicleFrame& f, InVehicle552Frame& cont) {
 Vehicle* old=f.oldInterface; Vehicle* next=f.newInterface; Object* m=f.movement;
 if (f.entering) {
  component_byte_14d(component_128(p)) |= 0x10;
  movement_vector_c4(m)=global_148218630(); movement_v460(m);
  movement_byte_1f0(m)&=0xfb; movement_vb00(m); pawn_flags_218(p)&=~2u;
  pawn_v798(p);
  Object* c=controller_if_142a3d9f0(controller(p));
  if (!c && vehicle_v568(next,state_seat(local_state(p)))) c=vehicle_v3b8(next,p);
  if (c) {
   vehicle_input(p)=vehicle_v48(next,state_seat(local_state(p)));
   if (vehicle_input(p)) { controller_vbd8(c,vehicle_input(p)); controller_input_7a0(c)=vehicle_input(p); }
   if (vehicle_v568(next,state_seat(local_state(p))) || vehicle_v560(next,state_seat(local_state(p)))) {
    pawn_vb68(p,0); pawn_vab0(p,0); fun_1423a8b00(c,1);
   }
  }
  Object* mesh=mesh_270(p);
  if (mesh) {
   pre_driving_anim_bp(p)=mesh_anim_class_628(mesh);
   Object* anim=vehicle_v2a0(next,state_seat(local_state(p)));
   Vehicle* current=pawn_v1088(p);
   if (!current || !vehicle_v568(current,state_seat(local_state(p)))) {
    if (anim && class_is_1446b91c0(anim)) fun_14234b2e0(p);
    fun_14234b3f0(p,current_vehicle_anim_layer(p),vehicle_v160(next));
   } else {
    if (vehicle_v178(next)) { Object* x=fun_144114bd0(mesh); if (x) fun_143fe17f0(x,0); }
    fun_14234b2e0_with_class(p,anim);
    fun_14234b3f0(p,current_vehicle_anim_layer(p),vehicle_v158(next));
   }
  }
  Vehicle* current=pawn_v1088(p);
  if (current && vehicle_v568(current,state_seat(local_state(p))) &&
      !fun_142331940(p) && !fun_14230c8b0(p) && !holster_contains(p,holster_driving())) pawn_vc60(p,true,0);
  if (footstep_surface(p)) {
   footstep_surface(p)=0;
   Object* target=event_target_v10(p);
   if (target) {
    EventTemp* event=event_temp_construct(p);
    event_set_surface_141e60520_143e8b3b0(event,footstep_surface(p));
    event_target_v620(target,event);
    for (int i=0, n=event_ref_count(event); i<n; ++i) release_event_ref(event_ref_at(event,i));
    if (event_heap_refs(event)) memory_free(event_heap_refs(event));
    if (event_heap_d8(event)) memory_free(event_heap_d8(event));
    if (event_heap_e8(event)) memory_free(event_heap_e8(event));
    if (event_heap_f8(event)) memory_free(event_heap_f8(event));
    if (event_heap_108(event)) memory_free(event_heap_108(event));
    event_restore_vtable(event); release_event_ref(event_shared_110(event)); event_temp_end_stack(event);
   }
  }
  fun_142a768d0(p); fun_142349360(p);
 }
 cont.seatController=nullptr;
 if (!f.seatChanged || !old) return InVehicleNext::AfterSeatChange;
 vehicle_v1e0(old);
 if (holster_contains(p,holster_148158d28())) pawn_vc60(p,false,0);
 vehicle_vd8(old,state_seat(local_state(p)),false);
 vehicle_vd8(old,state_seat(rep_state(p)),true);
 movement_byte_1f0(m)&=0xfb; pawn_flags_218(p)&=~2u;
 Object* root=vehicle_actor_component_130(state_vehicle(local_state(p)));
 if (root && object_class_is_1447c8590(root)) {
  Vec3 from=vehicle_v98_translation(old,state_seat(local_state(p)),0);
  Vec3 to=vehicle_v98_translation(old,state_seat(rep_state(p)),0);
  state_set_vector_10(local_state(p),{to.x-from.x,to.y-from.y,to.z-from.z});
 }
 Object* o=vehicle_v3b8(old,p); if (o) fun_1423ec450(o,state_seat(rep_state(p)));
 if (!vehicle_v568(old,state_seat(rep_state(p)))) fun_1420d8580(m); else fun_1420d8560();
 if (vehicle_vc8(old,state_seat(rep_state(p)))) {
  fun_144124090(component_280(p),true); movement_byte_38c(m)|=0x40;
 }
 Vec3 seat=vehicle_ve0(old,state_seat(rep_state(p)));
 uintptr_t attachment=vehicle_v90(old);
 pawn_v758(p,attachment,seat,true);
 fun_143f69330(p,state_vehicle(local_state(p)),dat_147cdd8d8(),seat);
 if (!vehicle_vc8(old,state_seat(rep_state(p)))) {
  fun_144124090(component_280(p),false); movement_byte_38c(m)&=0xbf;
 }
 if (!state_vehicle(local_state(p))) return InVehicleNext::DoneEarly;
 Vec3 transform=vehicle_va0(old,state_seat(local_state(p)),p);
 fun_144122860(component_128(p),transform,dat_147c31d50(),false); movement_vb00(m);
 Object* c=controller_if_142a3d9f0(controller(p));
 if (!c && f.newSeatDriving) c=vehicle_v3b8(old,p);
 if (c) { cont.seatController=c; return InVehicleNext::SeatControllerInput; }
 if (f.newSeatDriving) return InVehicleNext::SeatControllerInput; // line 881 skips input and enters driver section
 old_driver_seat_transition(p,f);
 return InVehicleNext::AfterSeatControllerInput;
}

// Source 857–1177: continuation and final shared callbacks.
extern void controller_vbe0(Object*);
extern void vehicle_vf8(Vehicle*,uint8_t); extern Object* vehicle_vb0(Vehicle*,uint8_t,uint8_t);
extern float pawn_v7b8(Pawn*,Object*,float,int);
extern float pawn_v7b8_old_driver_unresolved(Pawn*); // decompiler omits call arguments at line 958
extern uint8_t& reliable_input_byte2(Pawn*); extern uint8_t& emote_random_byte18(Pawn*);
struct Delegate; struct TimerArg; struct TimerCleanup; struct DelegateFactoryResult;
extern DelegateFactoryResult* delegate_factory_1406a86f0(Pawn*,uintptr_t,float);
extern Delegate* factory_get_cloneable(DelegateFactoryResult*); extern bool factory_has_value(DelegateFactoryResult*);
extern void factory_destroy_140692d60(DelegateFactoryResult*);
// Allocates 0x30; base vtable DAT_1459a9e80, weak pawn, callback, float
// payload, FUN_142bad740 result at +0x28, final vtable DAT_1459a9ed8.
extern Delegate* delegate_alloc_callback(Pawn*,uintptr_t,float);
extern Delegate* delegate_clone_v40(Delegate*); extern void delegate_destroy_v38(Delegate*);
extern void* memory_realloc_zero(void*);
extern void* timer_manager_140f95ae0(Pawn*); extern TimerArg* timer_arg_init_142ee3b30();
extern void timer_schedule_1446278d0(void*,Pawn*,int,Delegate*,float,TimerArg*);
// TimerArg contains a local or heap cleanup object; destroy v10 iff its native
// lStack_128 field is nonzero. The cloned delegate has a separate allocation
// and native active/count field, exposed below, before its storage is freed.
extern bool timer_arg_has_cleanup(TimerArg*); extern void timer_arg_cleanup_v10(TimerArg*);
extern void timer_arg_end_stack(TimerArg*); extern bool delegate_clone_active(Delegate*);
extern void vehicle_v1a8_seat(Vehicle*,Pawn*,uint32_t);
extern void fun_14235bfa0(Pawn*,Vehicle*); extern uint8_t state_byte_d(const VehicleState&);
extern Object* vehicle_ve8(Vehicle*,uint8_t,uint8_t); extern Object* fun_1423254c0(Pawn*);
extern Object* vehicle_v18(Vehicle*); extern uint8_t object_byte_a0(Object*);
extern void fun_143f84030(Pawn*,Vec3); extern void vehicle_v88(Vehicle*,Pawn*,Object*);
extern bool fun_141754730(Object*); extern void fun_1406a9dc0(Object*,int,int); // object, byte offset, argument
extern void vehicle_v80(Vehicle*,Pawn*,uint8_t); extern bool fun_1422fca30(Object*);
extern Object* object_ptr_250(Object*); extern void fun_1423ae530(Object*,int);
extern void weapon_v8b8(Object*,int); extern void broadcast_vehicle_change(Pawn*,int,Object*,Object*); // offset +20 or DAT_1478de578
extern bool runtime_options_byte_921(); extern Object* vehicle_v8(Vehicle*);
extern bool fun_1422fc9f0(Object*); extern void fun_1406934e0(Pawn*,bool);

// The native delegate clone and source have distinct lifetimes. For the first
// callback the source is held by the factory result; for the other two it is
// directly allocated. Keep the timer argument's v10 cleanup before delegate
// teardown, including realloc-to-zero followed by free if realloc returns data.
static void schedule_seat_transition(Pawn* p, int handleOffset, uintptr_t callback,
                                     float duration, bool factory) {
 void* manager=timer_manager_140f95ae0(p);
 DelegateFactoryResult* result=nullptr; Delegate* source=nullptr;
 if (factory) {
  result=delegate_factory_1406a86f0(p,callback,0.0f);
  if (factory_has_value(result)) source=factory_get_cloneable(result);
 } else source=delegate_alloc_callback(p,callback,0.0f);
 Delegate* clone=source ? delegate_clone_v40(source) : nullptr;
 TimerArg* arg=timer_arg_init_142ee3b30();
 timer_schedule_1446278d0(manager,p,handleOffset,clone,duration,arg);
 if (timer_arg_has_cleanup(arg)) timer_arg_cleanup_v10(arg);
 timer_arg_end_stack(arg);
 if (clone) {
  if (delegate_clone_active(clone)) { delegate_destroy_v38(clone); clone=(Delegate*)memory_realloc_zero(clone); }
  if (clone) memory_free(clone);
 }
 if (factory) factory_destroy_140692d60(result);
 else if (source) { delegate_destroy_v38(source); void* remain=memory_realloc_zero(source); if (remain) memory_free(remain); }
}

static void OnRep_InVehicle_857_1105(Pawn* p,InVehicleFrame& f,InVehicleNext nextStep,InVehicle552Frame& continuation) {
 Vehicle* old=f.oldInterface;
 if (nextStep==InVehicleNext::SeatControllerInput) {
  Object* c=continuation.seatController;
  if (c) {
   if (vehicle_input(p)) { controller_vbe0(c); vehicle_input(p)=nullptr; controller_input_7a0(c)=nullptr; }
   vehicle_input(p)=vehicle_v48(old,state_seat(rep_state(p)));
   if (vehicle_input(p)) { controller_vbd8(c,vehicle_input(p)); controller_input_7a0(c)=vehicle_input(p); }
   if (vehicle_v568(old,state_seat(rep_state(p))) || vehicle_v560(old,state_seat(rep_state(p)))) {
    pawn_vb68(p,0); fun_1423a8b00(c,1);
   }
  }
  if (f.newSeatDriving) {
   if (!vehicle_v560(old,state_seat(rep_state(p))) && !holster_contains(p,holster_driving())) pawn_vc60(p,true,0);
   Object* mesh=mesh_270(p);
   if (mesh) {
    Object* anim=vehicle_v2a0(old,state_seat(rep_state(p)));
    if (vehicle_v178(old)) { Object* x=fun_144114bd0(mesh); if (x) fun_143fe17f0(x,0); }
    fun_14234b2e0_with_class(p,anim);
    fun_14234b3f0(p,current_vehicle_anim_layer(p),vehicle_v158(old));
   }
  } else old_driver_seat_transition(p,f);
 }
 if (role(p)!=3) {
  if (role(p)==2) vehicle_v50(old,crouch_mesh_offset4(p));
  vehicle_vf8(old,state_seat(rep_state(p)));
  Object* montage=vehicle_vb0(old,state_seat(local_state(p)),state_seat(rep_state(p)));
  if (montage) {
   bool newDriving=vehicle_v568(old,state_seat(rep_state(p)));
   if (!newDriving) {
    bool oldDriving=vehicle_v568(old,state_seat(local_state(p)));
    float duration=oldDriving ? pawn_v7b8_old_driver_unresolved(p) : pawn_v7b8(p,montage,1.0f,0);
    if (duration>0) {
     if (!oldDriving) {
      reliable_input_byte2(p)|=4;
      schedule_seat_transition(p,8,0x14233ea80ULL,duration,true);
     } else {
      reliable_input_byte2(p)|=8;
      schedule_seat_transition(p,0x10,0x1423123e0ULL,duration,false);
     }
    }
   } else {
    float duration=pawn_v7b8(p,montage,1.0f,0);
    if (duration>0) {
     emote_random_byte18(p)=1;
     schedule_seat_transition(p,0x20,0x14233ea70ULL,duration,false);
    }
   }
   fun_142349360(p);
  }
 }
 copy_vehicle_state_32(local_state(p),rep_state(p));
 if (f.oldSeat!=UINT32_MAX) vehicle_v1a8_seat(old,p,f.oldSeat);
 vehicle_v70(old,state_seat(rep_state(p)),true);
}

static void OnRep_InVehicle_shared(Pawn* p,InVehicleFrame& f) {
 Vehicle* old=f.oldInterface; Vehicle* next=f.newInterface;
 fun_14235bfa0(p,next);
 if (old) {
  Object* result=vehicle_ve8(old,state_seat(rep_state(p)),state_byte_d(rep_state(p)));
  if (f.oldByteE==f.newByteE) {
   if (fun_1423254c0(p) && object_byte_a0(vehicle_v18(old))) fun_143f84030(p,{1,1,1});
   vehicle_v88(old,p,result);
  }
  Object* c=controller(p);
  if (!c || !fun_141754730(c)) {
   c=nullptr;
   if (controlled_rc_valid(p)) { Object* remote=controlled_rc_controller(p); if (remote && fun_141754730(remote)) c=remote; }
  }
  if (c) fun_1406a9dc0(c,0xa30,0);
 }
 if (next) {
  vehicle_v80(next,p,state_seat(local_state(p)));
  Object* c=controller(p);
  if (!c || !fun_141754730(c)) {
   Object* v=state_vehicle(rep_state(p)); c=nullptr;
   if (v && fun_1422fca30(v)) { Object* remote=object_ptr_250(v); if (remote && fun_141754730(remote)) c=remote; }
  }
  if (c) {
   fun_1423ae530(c,0);
   Object* w=current_weapon(p); if (w) weapon_v8b8(w,0);
   bool driving=vehicle_v568(next,state_seat(local_state(p)));
   fun_1406a9dc0(c,driving ? 0xa10 : 0xa20,0);
  }
 }
 broadcast_vehicle_change(p,0,f.newVehicle,f.oldVehicle);
 broadcast_vehicle_change(p,1,f.newVehicle,f.oldVehicle);
 if (runtime_options_byte_921()) {
  if (f.entering && next) { Object* v=vehicle_v8(next); if (fun_1422fc9f0(v) && v) fun_1406934e0(p,true); }
  if (f.leaving && old) { Object* v=vehicle_v8(old); if (fun_1422fc9f0(v) && v) fun_1406934e0(p,false); }
 }
}

void OnRep_InVehicle_rough(Pawn* p) {
 InVehicleFrame frame;
 if (!OnRep_InVehicle_prefix(p,frame)) return;
 OnRep_InVehicle_257_551(p,frame);
 InVehicle552Frame continuation;
 InVehicleNext cursor=OnRep_InVehicle_552_856(p,frame,continuation);
 if (cursor==InVehicleNext::DoneEarly) return;
 if (cursor!=InVehicleNext::AfterSeatChange) OnRep_InVehicle_857_1105(p,frame,cursor,continuation);
 OnRep_InVehicle_shared(p,frame);
}
