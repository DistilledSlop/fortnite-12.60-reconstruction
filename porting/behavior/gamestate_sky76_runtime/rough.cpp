// UNCOUNTED source lift. Not a native build input; every extern is unresolved.
#include <cstdint>
namespace sky76 {
using U=std::uint64_t; using I=std::int32_t; using B=std::uint8_t;
template<class T> T& at(U p,U o=0){return *reinterpret_cast<T*>(p+o);}
template<U Address,class... A> U call(A...);
template<class... A> U virt(U object,unsigned slot,A...);
extern U member_TotalFinalCountdownTime(U); // named Ghidra field addresses only
extern U member_PlayerBotsLeft(U);
extern U global_148263a18();
extern I global_148263a2c();
extern void finish_frame(U);
extern bool Stub_ReturnFalse(); // exact named source call; address not recovered here
extern void free_memory(U);
extern void array8_grow(U,I);
// Raw native stack98..stack40 span, not a verified engine iterator type.
struct Iterator {
 alignas(8) B bytes[96];
 U ptr(){return reinterpret_cast<U>(bytes);}
 template<class T> T& field(unsigned o){return at<T>(ptr(),o);}
};
static bool class_matches(U object,U type){
 U cls=at<U>(object,0x10); I index=at<I>(type,0x38);
 return at<I>(cls,0x38)>=index && at<U>(at<U>(cls,0x30),U(index)*8)==type+0x30;
}
static void cleanup(Iterator& it){
 if(it.field<B>(88)){
  it.field<B>(88)=0;
  call<0x14469a960>(it.field<U>(8),it.field<U>(80));
  if(it.field<U>(56))free_memory(it.field<U>(56));
  if(it.field<U>(16))free_memory(it.field<U>(16));
 }
}
// The two source loops differ only in which count local is named iVar13/16.
// Counts are reloaded after every rejected non-null entry, preserving mutation.
static void next_sky_actor(Iterator& it){
 U world=it.field<U>(8); I index=it.field<I>(32)+1;
 I count1=it.field<I>(24),count2=it.field<I>(64);
 while(index<count1+count2){
  U entry=index<count1 ? at<U>(it.field<U>(16),U(index)*8)
                       : at<U>(it.field<U>(56),U(index-count1)*8);
  ++it.field<I>(40);
  if(entry){
   U actor=call<0x143f76730>(entry);
   bool eligible=actor!=0;
   if(eligible && (it.field<std::uint32_t>(0)&1)){
    I object_index=at<I>(entry,0xc); U item=0;
    if(object_index<global_148263a2c()){
     std::uint32_t sign=std::uint32_t(object_index>>31)&0xffff;
     std::uint32_t adjusted=std::uint32_t(object_index)+sign;
     I chunk=I(adjusted)>>16;
     I within=I((adjusted&0xffff)-sign);
     item=at<U>(global_148263a18(),U(chunk)*8)+U(within)*0x18;
    }
    // Native output dereferences item even if out-of-range lookup yielded0.
    if((at<std::uint32_t>(item,8)>>29)&1)eligible=false;
   }
   if(eligible && ((it.field<std::uint32_t>(0)>>1)&1))
    eligible=Stub_ReturnFalse();
   if(eligible && ((it.field<std::uint32_t>(0)>>2)&1)){
    bool active=(at<B>(actor,500)&0x20)||(at<B>(actor,0x1f5)&0x40);
    U level=at<U>(actor,0x248);
    bool same=true;
    if(at<U>(actor,0xb8)){
     U current=call<0x14468e190>();
     same=!current || level==current;
    }
    bool level_ready=!level || at<B>(level)==2;
    if(!active || !(same || level_ready))eligible=false;
   }
   if(eligible && call<0x1413064f0>(actor)==world){
    if(actor==at<U>(world,0x30) || !class_matches(entry,call<0x14480eb00>())){
     it.field<I>(32)=index;it.field<U>(48)=entry;return;
    }
   }
   count1=it.field<I>(24);count2=it.field<I>(64);
  }
  ++index;
 }
 it.field<U>(48)=0;it.field<B>(36)=1;
}
void OnRep_SkyTubesShuttingDown(U self,U frame){
 finish_frame(frame);
 U countdown=member_TotalFinalCountdownTime(self);
 bool shutting_down=at<B>(countdown,6)!=0;
 if(shutting_down){
  auto value=std::uint32_t(call<0x1426feff0>());
  B authority=at<B>(self,0xf0);
  at<std::uint32_t>(countdown,0xc)=value;
  at<std::uint32_t>(countdown,8)=value;
  if(authority==3)return;
 }else if(at<B>(self,0xf0)==3)return;
 U world=virt(self,0x150);Iterator it;
 if(shutting_down){
  if(!at<B>(countdown,7)){
   U type=call<0x142ad7c80>();call<0x14197f340>(it.ptr(),world,type,5);
   while(!it.field<B>(36)){
    if(!at<B>(it.field<U>(48),0x218))call<0x142707ce0>();
    call<0x1410856e0>(it.ptr());
   }
   cleanup(it);
  }else{
   at<std::uint32_t>(countdown,8)=0x38d1b717;
   call<0x1419af2c0>(self,0);
  }
 }else{
  U bots=member_PlayerBotsLeft(self);
  at<I>(bots,0xc)=0;
  if(at<I>(bots,0x10))call<0x1406d9a30>(bots+4,0);
 }
 U type=call<0x142ad81d0>();call<0x14197f400>(it.ptr(),world,type,5);
 while(!it.field<B>(36)){
  U current=it.field<U>(48);
  if(!at<B>(current,0xb99)){
   if(shutting_down){
    U weak;call<0x142ee3b30>(reinterpret_cast<U>(&weak),current);
    U bots=member_PlayerBotsLeft(self);I count=at<I>(bots,0xc);
    at<I>(bots,0xc)=count+1;
    if(at<I>(bots,0x10)<count+1)array8_grow(bots+4,count);
    at<U>(at<U>(bots,4),U(count)*8)=weak;
   }else call<0x142706450>(current,1);
  }
  next_sky_actor(it);
 }
 cleanup(it);
}
} // namespace sky76
