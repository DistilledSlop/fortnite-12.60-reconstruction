// UNCOUNTED AFortPlayerStateAthena source draft, not a native input.
#include <cstdint>
namespace metric76 {
using U=std::uint64_t;using I=std::int32_t;using B=std::uint8_t;
template<class T>T& at(U p,U o=0){return *reinterpret_cast<T*>(p+o);}
template<U Address,class...A>U call(A...);
extern void finish_frame(U);
extern U member_MetricInformation(U);
extern U member_PartyAssistQuestTemplate(U);
extern void free_memory(U);
// Both source memcpy calls omit their third argument. Do not invent a length.
extern void memcpy_native_missing_length(U destination,U source);
void OnRep_MetricInformation(U self,U frame){
 finish_frame(frame);
 struct Payload{U self,data,count_capacity;} payload{self,0,0};
 U local=reinterpret_cast<U>(&payload.data),source=member_MetricInformation(self);
 if(local!=source){
  U data=at<U>(source);std::uint32_t count=at<std::uint32_t>(source,8);
  payload.count_capacity=count;
  if(count){
   call<0x1406909c0>(local,count,0);
   U input=data,output=payload.data;
   do{
    at<U>(output)=0;
    I n=at<I>(input,8);U text=at<U>(input);
    at<I>(output,8)=n;
    if(!n)at<I>(output,12)=0;
    else{call<0x140690ac0>(output,n,0);memcpy_native_missing_length(at<U>(output),text);}
    at<U>(output,16)=0;
    n=at<I>(input,24);text=at<U>(input,16);
    at<I>(output,24)=n;
    if(!n)at<I>(output,28)=0;
    else{call<0x140690ac0>(output+16,n,0);memcpy_native_missing_length(at<U>(output,16),text);}
    at<B>(output,32)=at<B>(input,32);
    at<std::uint32_t>(output,36)=at<std::uint32_t>(input,36);
    at<std::uint32_t>(output,40)=at<std::uint32_t>(input,40);
    input+=48;output+=48;
   }while(--count);
  }
 }
 call<0x1406a9dc0>(member_PartyAssistQuestTemplate(self)+8,reinterpret_cast<U>(&payload));
 U item=payload.data;
 for(I n=I(payload.count_capacity);n!=0;--n,item+=48){
  if(at<U>(item,16))free_memory(at<U>(item,16));
  if(at<U>(item))free_memory(at<U>(item));
 }
 if(payload.data)free_memory(payload.data);
}
} // namespace metric76
