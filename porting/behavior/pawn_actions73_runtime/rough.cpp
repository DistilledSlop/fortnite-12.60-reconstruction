// Rough connected lifts of six actual native bodies. Adapters are unresolved contracts.
#include <cstdint>
#include <cstring>
#include <cmath>
#include <xmmintrin.h>
namespace pawn_actions73 {
using u8 = std::uint8_t; using u32 = std::uint32_t; using u64 = std::uint64_t;
using Ptr = std::uintptr_t;
template<class T> T read(Ptr p, std::size_t o = 0) { T v; std::memcpy(&v, reinterpret_cast<void*>(p+o), sizeof v); return v; }
template<class T> void write(Ptr p, std::size_t o, T v) { std::memcpy(reinterpret_cast<void*>(p+o), &v, sizeof v); }
struct Vec3 { float x,y,z; };
struct Raw12 { u64 a; u32 b; };
extern Ptr get_current_montage(); // 14070c910
extern void montage_next_section(Ptr anim, u64 from, u64 to); // vtable +760
extern Ptr pawn_v150(Ptr); extern Ptr pawn_v628(Ptr); extern void pawn_v5e8(Ptr);
extern Ptr knockback_type(); extern Ptr movement_type(); // 142927a60, 1429f5d30
extern void movement_v420(Ptr); extern void movement_v590(Ptr,int);
extern Vec3 small_vector_fallback(); // DAT_148218630..638, unresolved data
extern bool fname_equals_none(u32); extern int attach_predicate(Ptr);
extern Ptr find_socket(Ptr,u64); extern Ptr find_or_add_attachment(Ptr,u64);
extern void finish_attachment(Ptr,Ptr);
extern u8 attach_log_verbosity();
// 142305f70 missing-socket logging: adapter must construct mesh name (or allocated
// "None" with the original 5-wide-character allocation/copy), convert pawn name,
// socket name, and the additional pawn-name conversion at the omitted-argument
// 142ce9f90 call; perform the level-3 log with its unresolved varargs ABI and free
// the four resulting non-null buffers in order: extra, socket, pawn, mesh.
// It must preserve allocation/growth and conversion side effects and cannot be a no-op.
extern void missing_socket_log_with_cleanup(Ptr pawn, Ptr mesh, Ptr attachment);
// The call's decompilation does not expose incoming direction (arg2), and its
// output stack layout/calling convention is unresolved. Adapter must resolve the
// native ABI, including arg2, and consume these exact matrix values.
extern void spherical_math_142c521d0(u64 incoming_direction, float out[2],
                                    float scalar80, const float row78[3],
                                    const float row88[3], const float row98[3]);
extern void empty_subobject_name_diagnostic(Ptr,const wchar_t*);
extern u64 death_socket_class(); extern Ptr create_death_socket(u64,Ptr,u64,u64,u64,u64,u64,u64,u64);
extern void configure_death_socket(Ptr,Ptr,const Raw12*,const Raw12*);
extern Ptr feedback_manager();
// Captures the five indeterminate prior-stack bytes that occupy bits 24..63.
extern u64 feedback_prior_stack_upper_five_bytes();
extern void dispatch_feedback(Ptr,const u64 payload[3],u64,u64,u32,int,int);

void AnimMontage_LocalSetNextSectionName(Ptr p, Ptr montage, u64 from, u64 to) {
  if (read<Ptr>(p,3000) && montage) {
    Ptr current = get_current_montage();
    if (montage == current) montage_next_section(read<Ptr>(p,3000),from,to);
  }
}
static bool is_a(Ptr object, Ptr type) {
  Ptr klass = read<Ptr>(object,0x10); int index = read<int>(type,0x38);
  return read<int>(klass,0x38) >= index && read<Ptr>(read<Ptr>(klass,0x30),static_cast<std::intptr_t>(index)*8) == type+0x30;
}
void ApplyKnockback(Ptr p, float strength, float angle, const float* direction) {
  Ptr owner = read<Ptr>(pawn_v150(p),0x130);
  if (owner && !is_a(owner,knockback_type())) owner = 0;
  if (read<u8>(p,0xf0) != 3 || !owner) return;
  float c = std::cos(angle * 0.017453294f);
  float x = direction[0], y = direction[1];
  float length2 = x*x + y*y, z;
  if (length2 == 1.0f) {
    if (direction[2] == 0.0f) { x=direction[0]; y=direction[1]; z=direction[2]; goto normalized; }
  } else {
    if (length2 < 1e-8f) { Vec3 v=small_vector_fallback(); x=v.x; y=v.y; z=v.z; goto normalized; }
    float r = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(length2)));
    r = r + r*(0.5f-length2*0.5f*r*r);
    r = r + r*(0.5f-length2*0.5f*r*r);
    y=y*r; x=x*r;
  }
  z=0.0f;
normalized:
  float s = std::sin(angle * 0.017453294f);
  z=(c*z+s)*strength; x=c*x*strength; y=c*y*strength;
  Ptr movement = pawn_v628(p);
  if (movement && is_a(movement,movement_type()) && 0.0f < z) {
    write<u8>(movement,0x124,read<u8>(movement,0x124)&0xf7);
    movement_v420(movement);
    write<u8>(movement,0x124,read<u8>(movement,0x124)|8);
    movement_v590(movement,3);
    write<float>(movement,0xc4,x); write<float>(movement,0xc8,y); write<float>(movement,0xcc,z);
    pawn_v5e8(p);
  }
}
void AttachAssetToSocket(Ptr p, Ptr a) {
  bool none=fname_equals_none(read<u32>(a));
  if ((!none || read<u32>(a,4)!=0) && read<Ptr>(p,0x278) &&
      (read<u8>(a,0x18)==0 || attach_predicate(p)!=1)) {
    Ptr socket=find_socket(read<Ptr>(p,0x278),read<u64>(a));
    if (!socket) {
      if (attach_log_verbosity()>2) missing_socket_log_with_cleanup(p,read<Ptr>(p,0x278),a);
    } else {
      Ptr dest=find_or_add_attachment(p,read<u64>(a));
      if (dest) {
        write<u64>(dest,0,read<u64>(a)); write<u64>(dest,8,read<u64>(a,8));
        write<u64>(dest,16,read<u64>(a,16)); write<u8>(dest,24,read<u8>(a,24));
        write<u8>(dest,25,read<u8>(a,25)); finish_attachment(p,dest);
      }
    }
  }
}
void GetSphericalHitDirection(Ptr p,u64 direction,float* out1,float* out2) {
  Ptr root=read<Ptr>(p,0x130); float x,y,z,w;
  if (!root) { x=0; y=0; z=0; w=1; }
  else { x=read<float>(root,0x1c0); y=read<float>(root,0x1c4); z=read<float>(root,0x1c8); w=read<float>(root,0x1cc); }
  float z2=z*2.0f, y2=y*2.0f, xx2=x*x*2.0f;
  float wx2=w*x*2.0f;
  float row78[3]={1.0f-(z*z2+y*y2),w*z2+x*y2,x*z2-w*y2};
  float row88[3]={x*y2-w*z2,1.0f-(z*z2+xx2),wx2+y*z2};
  float row98[3]={w*y2+x*z2,y*z2-wx2,1.0f-(y*y2+xx2)};
  float out[2]; spherical_math_142c521d0(direction,out,row88[2],row78,row88,row98);
  *out1=out[0]*57.295776f; *out2=out[1]*57.295776f;
}
void InitializeDeathHitSocket(Ptr p,const Raw12* a,const Raw12* b) {
  Ptr socket=read<Ptr>(p,0x6a0);
  if (!socket) {
    empty_subobject_name_diagnostic(p,L"NewObject with empty name can't be used to create default subobjects (inside of UObject derived class constructor) as it produces inconsistent object names. Use ObjectInitializer.CreateDefaultSuobject<> instead.");
    u64 klass=death_socket_class(); socket=create_death_socket(klass,p,0,0,0,0,0,0,0);
    write<Ptr>(p,0x6a0,socket); if (!socket) return;
  }
  if (read<Ptr>(p,0x278)) {
    Raw12 local_b{b->a,b->b}, local_a{a->a,a->b};
    configure_death_socket(socket,read<Ptr>(p,0x278),&local_a,&local_b);
  }
}
void TriggerFeedbackEvent(u64 /*unused*/,u64 p2,u64 p3,u64 p4,u32 p5) {
  Ptr manager=feedback_manager();
  if (manager) {
    u64 prior=feedback_prior_stack_upper_five_bytes();
    u64 payload[3]={0,p2,((prior & 0xffffffffffULL)<<24)|0x50000ULL};
    dispatch_feedback(manager,payload,p3,p4,p5,1,1);
  }
}
}
