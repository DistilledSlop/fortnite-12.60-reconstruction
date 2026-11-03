// Rough behavioral lift of 0x1426aafb0. Adapter declarations model unresolved ABI;
// this is evidence, not a buildable replacement. Offsets are byte offsets.
#include <cstdint>
#include <initializer_list>
using U = std::uint64_t; using I = std::int32_t; using B = std::uint8_t;
template<class T> T& at(U p, U off) { return *reinterpret_cast<T*>(p + off); }
template<U Address, class... A> U call(A...); // external address-specific ABI adapter
template<class... A> U virt(U object, U slot, A...); // dispatch through object vtable
extern U resize(U array, I oldCount); extern void free_mem(U); extern U realloc_mem(U,U,U);
extern void compact(U, bool); extern void clear_timer(U,U);
extern U empty_string_1459a4b30;
// Models the native stack value at cleanup; its producer and ABI are unresolved.
extern U unresolved_native_stack98(U key, U scratch80);
struct Array { U data; I count; I capacity; };
struct PairArrays { Array a, b; };
// The decompiler uses overlapping stack slots in the later delegate sequence.
struct DelegateScratch { U slot98, slot90, slot88, slot80, slot78, slot70, slot68, slot60; };

void weapon_1426aafb0(U w) {
  at<B>(w,0x9c4)=0;
  U scratch; at<B>((U)&scratch,0)=0; // remaining incoming stack-home bytes unspecified
  call<0x1406a9dc0>(w+0x2f0,&scratch);
  virt(w,0x758,w);
  if (!B(call<0x141ed8de0>(w)) || !at<B>(w,0x380) || at<I>(w,0x940)>0 || at<I>(w,0x950)>0)
    call<0x1426aeed0>(w);
  else call<0x142694dd0>(w);
  I i=0; I n=(I)virt(at<U>(w,0x798),0x6a0);
  if (n>0) {
    U pos=0;
    do {
      if (at<I>(w,0x7b8)<=i) break;
      U target=at<U>(w,0x798);
      virt(target,0x550,target,i,at<U>(at<U>(w,0x7b0),pos));
      ++i; pos+=8;
      n=(I)virt(at<U>(w,0x798),0x6a0);
    } while(i<n);
  }
  at<I>(w,0x7b8)=0;
  if (at<I>(w,0x7bc)!=0) call<0x1406d9a30>(w+0x7b0,0);
  U owner;
  if (at<U>(w,0x350) && (owner=call<0x141125940>(w))) {
    U cls=call<0x142a75f10>(); U ownerClass=at<U>(owner,0x10); I index=at<I>(cls,0x38);
    if (index<=at<I>(ownerClass,0x38) && at<U>(at<U>(ownerClass,0x30),U(index)*8)==cls+0x30) {
      PairArrays source{}, tags{};
      U component=at<U>(w,0x350)+0x30;
      virt(component,0x10,component,&source);
      U v=call<0x142323510>(owner,0);
      v=call<0x141d4ab00>(v,&source);
      call<0x141d4dc30>(v,&tags);
      U manager=virt(owner+0x4c8,0x10);
      if (manager) {
        Array selected{};
        if (at<I>(w,0x390)>0) call<0x1406d9a30>(&selected);
        i=0; U data=selected.data; I count=selected.count;
        while (i>=0 && i<at<I>(w,0x390)) {
          U input=at<U>(w,0x388);
          if (B(call<0x140744d30>(manager+0x1040))) {
            selected.count=count+1;
            if (selected.capacity<selected.count) { resize((U)&selected,count); data=selected.data; }
            at<U>(data,U(count)*8)=at<U>(input,U(i)*8);
            count=selected.count;
          }
          ++i;
        }
        if (count>0) call<0x143e8f0b0>(manager+0x1148);
        for (U p=data; p!=data+U(count)*8; p+=8) virt(manager,0x810,manager,p,0);
        if (data) free_mem(data);
        call<0x143e96ff0>(w+0x388,0);
        if (tags.a.count!=0) {
          PairArrays extra{}; i=0;
          while(i>=0 && i<tags.a.count) {
            scratch=at<U>(tags.a.data,U(i)*8);
            call<0x141ed2350>(); U registry=call<0x141ed1800>();
            if (B(call<0x143e93680>(&scratch,registry+0x1918))) {
              Array name; call<0x142ce9f90>(&scratch,&name);
              U s=name.count ? name.data : empty_string_1459a4b30;
              Array formatted; call<0x142bafb80>(&formatted,L"%s.Weapon",s);
              s=formatted.count ? formatted.data : empty_string_1459a4b30;
              U temporary1,temporary2;
              v=call<0x142cd4ab0>(&temporary1,s,1);
              v=call<0x143e96810>(&temporary2,v);
              call<0x143e8b3b0>(&extra,v);
              if (formatted.data) free_mem(formatted.data);
              if (name.data) free_mem(name.data);
            }
            ++i;
          }
          call<0x143e8c1b0>(&tags,&extra);
          selected={};
          if (tags.a.count>0) call<0x1406d9a30>(&selected,(U)(std::uint32_t)tags.a.count);
          I j=0; data=selected.data; n=tags.a.count; count=selected.count;
          while(j>=0 && j<n) {
            U element=tags.a.data+U(j)*8;
            if (B(call<0x140744d30>(manager+0x1040))) {
              selected.count=count+1;
              if (selected.capacity<selected.count) { resize((U)&selected,count); data=selected.data; }
              at<U>(data,U(count)*8)=at<U>(element,0);
              count=selected.count;
            }
            ++j; n=tags.a.count;
          }
          if (count>0) call<0x143e8f0b0>(manager+0x1148);
          for(U p=data; p!=data+U(count)*8; p+=8) virt(manager,0x810,manager,p,0);
          if(data) free_mem(data);
          if(extra.b.data) free_mem(extra.b.data);
          if(extra.a.data) free_mem(extra.a.data);
        }
      }
      if(tags.b.data) free_mem(tags.b.data);
      if(tags.a.data) free_mem(tags.a.data);
      if(source.b.data) free_mem(source.b.data);
      if(source.a.data) free_mem(source.a.data);
    }
  }
  for(U p=w+0x768, remaining=3; remaining; p+=8,--remaining) {
    U obj=at<U>(p,0); if(obj) virt(obj,0x528,obj,0x3e19999a,0,0);
  }
  for(U p=w+0x780, remaining=2; remaining; p+=8,--remaining) {
    U obj=at<U>(p,0); if(obj) virt(obj,0x528,obj,0x3e19999a,0,0);
  }
  U world=virt(w,0x150,w);
  if(world) {
    U sub=at<U>(world,0x188); U timers=sub ? at<U>(sub,0xc0) : at<U>(world,0x320);
    for(U offset : {U(0x9c8),U(0xb30),U(0x280)}) {
      if(call<0x144621fb0>(timers,w+offset)) clear_timer(timers,at<U>(w,offset));
      at<U>(w,offset)=0;
    }
  }
  at<B>(w,0x288)=0;
  std::uint32_t time=world ? at<std::uint32_t>(world,0x540) : 0xbf800000;
  at<std::uint32_t>(w,0x8d8)=at<std::uint32_t>(w,0x8d4);
  at<std::uint32_t>(w,0x8bc)=time;
  at<B>(w,0xb49)=0; at<std::uint32_t>(w,0x8e4)=0; at<std::uint32_t>(w,0x8dc)=0;
  call<0x142b43850>(w);
  U pawn=call<0x1426a07c0>(w);
  if(pawn) {
    U cls=call<0x142a3d9f0>(); U pc=at<U>(pawn,0x10); I index=at<I>(cls,0x38);
    if(at<I>(pc,0x38)<index || at<U>(at<U>(pc,0x30),U(index)*8)!=cls+0x30) pawn=0;
    else {
      U v=call<0x14239b310>(pawn,0); call<0x140690620>(v,w);
      U context, manager;
      if(at<U>(w,0x9d0) && (context=virt(pawn,0x14d8,pawn)) && (manager=virt(context+0x4c8,0x10))) {
        v=call<0x142b493b0>();
        if(call<0x1406c7d70>(manager,v)) {
          PairArrays key; DelegateScratch temp;
          call<0x141629b80>(&key);
          U match=at<U>(w,0x9d0);
          v=call<0x1406aade0>(&temp.slot80,&key);
          U list=call<0x1406c90d0>(manager,v);
          if(match && at<I>(list,8)>0) {
            i=0; U pos=0;
            do {
              U base=at<U>(list,0); U entry=base+pos;
              if(at<I>(entry,8) && at<U>(entry,0)) {
                U result=virt(at<U>(entry,0),0x30);
                if(at<U>(result,0)==match) {
                  if(at<I>(entry,8) && at<U>(entry,0)) {
                    virt(at<U>(entry,0),0x38);
                    if(at<U>(entry,0)) at<U>(entry,0)=realloc_mem(at<U>(entry,0),0,0);
                    at<I>(entry,8)=0;
                  }
                  compact(list,false); break;
                }
              }
              ++i; pos+=0x10;
            } while(i<at<I>(list,8));
          }
          // The decompiler does not identify the producer of stack -0x98.
          U stack98=unresolved_native_stack98((U)&key,(U)&temp.slot80);
          if(stack98) free_mem(stack98);
          if(key.a.data) free_mem(key.a.data);
        }
      }
    }
  }
  if(at<std::uint32_t>(w,0x900)&8) {
    at<std::uint32_t>(w,0x3d8)=0; at<std::uint32_t>(w,0x900)&=~std::uint32_t(8);
    virt(w,0x690,w); virt(w,0xa60,w,0); call<0x142694ec0>(w);
  }
  owner=call<0x141125940>(w);
  if(owner) {
    U cls=call<0x142a75f10>(); U oc=at<U>(owner,0x10); I index=at<I>(cls,0x38);
    if(index>at<I>(oc,0x38) || at<U>(at<U>(oc,0x30),U(index)*8)!=cls+0x30) owner=0;
  }
  if(!at<B>(w,0x288) && pawn && at<B>(pawn,0xf0)==3 && owner && at<U>(w,0x350) && at<I>(at<U>(w,0x350),0x970)>0) {
    U controller=at<U>(pawn,0x220);
    if(controller) {
      U cls=call<0x142a85850>(); U cc=at<U>(controller,0x10); I index=at<I>(cls,0x38);
      if(index<=at<I>(cc,0x38) && at<U>(at<U>(cc,0x30),U(index)*8)==cls+0x30) {
        call<0x142408830>(controller,pawn,owner); virt(controller,0x5e8,controller);
      }
    }
  }
  scratch=w; call<0x1406a9dc0>(w+0x290,&scratch);
}
