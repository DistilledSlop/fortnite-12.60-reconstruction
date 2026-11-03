// Uncounted behavioral sketches; adapters are unresolved, not executable implementations.
// Offsets below are from raw decomp; do not treat inferred types as verified ABI.
#include <cstdint>
struct Obj; struct Pawn; struct Vec3 { float x,y,z; }; struct Tags;
extern uint8_t& b(void*, unsigned); extern float& f(void*, unsigned);
extern Obj*& obj(void*, unsigned); extern void* at(void*, unsigned);
extern uint64_t& q(void*,unsigned); extern uint32_t& d(void*,unsigned);
extern bool pawn_6b0(Pawn*); extern void pawn_b68(Pawn*, int); extern void pawn_1018(Pawn*);
extern Obj* pawn_150(Pawn*); extern bool call_144693a30(Obj*); extern Obj* call_142321a70(Pawn*);
extern void call_142a77180(Pawn*); extern void call_140684de0(void*);
extern void broadcast_1406a9dc0(void* delegate, const void* args);
extern void call_1426b3b20(Obj*, bool, bool); extern bool weapon_828(Obj*, int, int);
extern int weapon_7f8(Obj*); extern void weapon_838(Obj*); extern void weapon_718(Obj*);
extern bool call_1426a5af0(Obj*); extern void call_142698da0(Obj*);
extern void movement_420(Obj*); extern void movement_568(Obj*, Vec3*, bool);
extern Vec3 normalization_global_148218630(); // three actual global floats; not an assumed zero
extern float native_rsqrt_two_refinements(float); // rsqrtss then two Newton steps, exact native FP behavior
extern Obj* call_144114bd0(); // receiver/arguments not recovered by decompiler
extern void* class_142b664d0(); extern void* class_142b5e190(); extern void* class_1427b4d30();
extern bool class_hierarchy_test(Obj*, void*); // class +0x38 index against object class's +0x30 table
extern Obj* safezone_context_4c8_10(Pawn*); extern void* globals_141ed1800();
extern int lookup_1406c6690(void* map, void* key); // returns index or -1
extern int map_entry_count(void* map, int index); // map data + index*0x14 + 8
extern void call_140744d30(void*, void*, int delta, int zero);
extern Obj* call_141680790(Pawn*); extern bool call_14241bb20(Obj*);
extern Vec3 global_148218640();
extern Obj* spawn_144230ed0(Obj* sound, Obj* component, int, Vec3*, Vec3*, int, int, float, float, int, int, int, int);
extern void audio_520(Obj*, float fade, float volume, int, uint64_t low_byte_zero_arg);
extern void call_1423526c0(Pawn*);
extern void* global_141e5b2a0(); extern void* call_141752740(void*, int);
extern void tags_empty(Tags&); extern void tags_add_143e8b3b0(Tags&, void*);
extern void tags_copy_143e8a690(Tags&, Tags&); extern void tags_free_gathered_arrays_if_nonnull(Tags&); // +0x10 then +0
extern void pawn_a78(Pawn*, Tags&, int, int);

void OnRep_PushMomentum(Pawn* p) {
 if (b(p,0xf0)==1 || !obj(p,0x280)) return;
 Obj* m=obj(p,0x280); float x=f(p,0x628), y=f(p,0x62c), z=f(p,0x630);
 float size=x*x+y*y+z*z;
 if (size<=0.0f) { b(m,0x124)&=0xf7; movement_420(m); b(m,0x124)|=8; }
 else {
  Vec3 direction;
  if (size==1.0f) direction={x,y,z};
  else if (1e-8f<=size) { float s=native_rsqrt_two_refinements(size); direction={x*s,y*s,z*s}; }
  else direction=normalization_global_148218630();
  f(m,0xc4)=x; f(obj(p,0x280),0xc8)=f(p,0x62c);
  movement_568(obj(p,0x280),&direction,true);
 }
}
void OnRep_bWeaponHolstered(Pawn* p) {
 Obj* w=obj(p,0x588);
 if (w && w==obj(p,0x6d8)) {
  call_1426b3b20(w,!(b(p,0x539)&0x10),true);
  if (!(b(p,0x539)&0x10)) {
   if (pawn_6b0(p)) {
    if (weapon_828(obj(p,0x588),0,0) && weapon_7f8(obj(p,0x588))==0) weapon_838(obj(p,0x588));
    weapon_718(obj(p,0x588));
   }
  } else { if (call_1426a5af0(obj(p,0x588))) call_142698da0(obj(p,0x588)); pawn_b68(p,0); }
 }
 broadcast_1406a9dc0(at(p,0xa08 | (unsigned)(~b(p,0x539)&0x10)),nullptr);
}
void KillDisconnectedPawn(Pawn* p, Tags& gathered, Tags& copy) {
 // Caller supplies uninitialized opaque storage; both containers are zero-initialized here.
 tags_empty(gathered); tags_add_143e8b3b0(gathered,at(globals_141ed1800(),0x440));
 void* found=call_141752740(at(global_141e5b2a0(),0xd98),1);
 if (found) tags_add_143e8b3b0(gathered,at(found,0x70));
 tags_empty(copy); tags_copy_143e8a690(copy,gathered); pawn_a78(p,copy,0,0);
 tags_free_gathered_arrays_if_nonnull(gathered); // no explicit cleanup of copy is shown
}
void OnRep_FacialTypeOverride(Pawn* p) {
 b(p,0x2728)=b(p,0x26b4);
 if (obj(p,0x18d0)) { Obj* a=call_144114bd0(); if (a && class_hierarchy_test(a,class_142b664d0())) { q(a,0x418)=q(p,0x2720); d(a,0x420)=d(p,0x2728); } }
 if (obj(p,0x278)) { Obj* a=call_144114bd0(); if (a && class_hierarchy_test(a,class_142b5e190())) { q(a,0x2d8)=q(p,0x2720); d(a,0x2e0)=d(p,0x2728); } }
}
void OnRep_IsInsideSafeZone(Pawn* p) {
 Obj* c=safezone_context_4c8_10(p);
 if (c) {
  uint8_t state=b(p,0x1220); void* key=at(globals_141ed1800(),0x928);
  int index=lookup_1406c6690(at(c,0x10e0),key);
  int previous=index==-1 ? 0 : map_entry_count(at(c,0x10e0),index);
  int delta=((state>>1)&1)-previous;
  if (delta) call_140744d30(at(c,0x1040),key,delta,0);
 }
 uint8_t arg=(b(p,0x1220)>>1)&1; broadcast_1406a9dc0(at(p,0x21e8),&arg);
}
void OnRep_IsInAnyStorm(Pawn* p) {
 Obj* world=pawn_150(p); bool condition;
 if (world && call_144693a30(world)) condition=(~b(p,0x1220)&1)!=0;
 else condition=call_142321a70(p)!=nullptr;
 uint8_t saved=b(p,0x1061);
 if (((saved&2) && !(b(p,0x1220)&1)) || condition) {
  b(p,0x1061)=(b(p,0x1220)&1) ? (saved|2) : (saved&0xfd);
  call_142a77180(p); call_140684de0(at(p,0x1238));
 }
 pawn_1018(p);
}
void OnRep_IsParachuteForcedOpen(Pawn* p) {
 if (!obj(p,0x25e8) && (b(p,0x10e7)&2) && (b(p,0x10e7)&4) && !pawn_6b0(p) && !call_142321a70(p)) {
  Obj* other=call_141680790(p);
  if (other && !call_14241bb20(other)) {
   Vec3 location={0,0,0}, rotation=global_148218640();
   Obj* a=spawn_144230ed0(obj(p,0x2620),obj(p,0x278),0,&location,&rotation,0,0,1.0f,1.0f,0,0,0,1);
   obj(p,0x25e8)=a;
   if (a) { audio_520(a,f(p,0x25f0),1.0f,0,reinterpret_cast<uintptr_t>(&rotation) & ~uintptr_t(0xff)); call_1423526c0(p); }
  }
 }
}
void OnRep_PossessedProp(Pawn* p) {
 struct Args { Pawn* pawn; Obj* prop; } args{p,obj(p,0x15e8)};
 broadcast_1406a9dc0(at(p,0x15d8),&args);
 if (!obj(p,0x278)) return;
 Obj* a=call_144114bd0();
 if (a && class_hierarchy_test(a,class_1427b4d30())) {
  if (obj(p,0x15e8)) b(a,0x340)&=0xfe; else b(a,0x340)|=1;
 }
}
