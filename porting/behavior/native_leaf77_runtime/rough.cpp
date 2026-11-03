// UNCOUNTED native dependency bodies; not canonical build inputs.
#include <cstdint>
namespace leaf77 {
using U=std::uint64_t;using B=std::uint8_t;using I=std::int32_t;
template<class T>T& at(U p,U o=0){return *reinterpret_cast<T*>(p+o);}
template<U Address,class... A>U call(A...);
// Vtable is selected from object, but the visible native argument list is empty.
extern U virtual_no_visible_arguments(U object,unsigned slot);
extern void virtual_item_2d0(U object,U output,U source);
extern I tls_epoch_248();
extern I& global_guard_1481134f8();
extern U& global_name_1481134f0();
extern U address_of_guard_1481134f8();
extern U address_of_name_1481134f0();
void SkyActorEnable(U actor,B value){ //142706450
 if(at<B>(actor,0xb98)==value)return;
 at<B>(actor,0xb98)=value;
 call<0x142ad9990>();
}
void SkyTubeShutdown(U actor){ //142707ce0
 if(!at<B>(actor,0x5c1) && !at<B>(actor,0x218)){
  at<B>(actor,0x5c1)=1;
  call<0x142ad99d0>();
 }
}
U SkyStateShutdown(U self,std::uint32_t raw_value){ //1419af2c0
 if(at<B>(self,0xf0)==3)return 0;
 U world=virtual_no_visible_arguments(self,0x150);
 if(!world || !at<U>(self,0x2d0))return 0;
 U instance=call<0x14468fb30>(world,at<U>(at<U>(self,0x2d0),0xca8));
 if(!instance)return 0;
 if(tls_epoch_248()<global_guard_1481134f8()){
  call<0x145803a44>(address_of_guard_1481134f8());
  if(global_guard_1481134f8()==-1){
   call<0x142cd4ab0>(address_of_name_1481134f0(),u"SlipstreamsAlpha",1);
   call<0x1458039e4>(address_of_guard_1481134f8());
  }
 }
 call<0x144366c30>(instance,global_name_1481134f0(),raw_value);
 return 1;
}
U ConvertItemQuantity(U output,U source,std::uint32_t quantity){ //141f7ce30
 U manager=call<0x141df62a0>();
 virtual_item_2d0(manager,output,source+0x10);
 at<std::uint32_t>(output,0x10)=quantity;
 return output;
}
B SharedMovementDifferent(const float* current,const float* incoming){ //141a0a860
 // Preserve short circuit order and native NaN inequality for all12 floats.
 for(unsigned i=0;i<12;++i)if(!(incoming[i]==current[i]))return 1;
 B a=*reinterpret_cast<const B*>(current+12);
 B b=*reinterpret_cast<const B*>(incoming+12);
 if((a^b)&1)return 1;
 return ((a^b)>>1)&1;
}
} // namespace leaf77
