// Source recovery for 0x1410f2740. ABI/layout adapters below are unresolved;
// this is a connected rough lift, not a drop-in engine implementation.
#include <cstdint>
#include <cstddef>
namespace profile_overlay81 {
using i32 = int32_t; using u32 = uint32_t; using u64 = uint64_t;
struct String; struct Entry; struct Overlay; struct Change; struct StatChange;
struct UObject; struct Shared { void* object; void* control; };
struct Frame { alignas(16) unsigned char bytes[0x400]; }; // individual native stack output frames
struct Sparse {
  void* slots; i32 count, capacity; u32 inlineBits[4]; u32* heapBits;
  i32 bitCount, bitCapacity, firstFree, freeCount; void* hash; i32 hashSize;
};
struct Hooks {
  UObject* weak_get(void* weak); i32 profile_state(UObject*); u32 read32(void*,size_t);
  void* ptr(void*,size_t); i32& int_at(void*,size_t); uint8_t& byte_at(void*,size_t);
  void init_accumulator(void*); void destroy_accumulator(void*); void free_mem(void*);
  Shared make_string_copy(const String*); Shared make_entry_copy(const void*); void normalize_shared(Shared*);
  void retain(Shared); void release(Shared); // non-atomic strong/weak release with virtual destruction
  i32 overlay_find_exact(void* map, const Shared* key); // case-sensitive text
  void overlay_alloc(void* map, Frame* output); void* output_slot(Frame*);
  void overlay_construct(void* slot, Shared key, const void* temporary);
  u32 hash_string(const String*); void overlay_insert(void* map, Frame* result, u32 hash, void* slot);
  void temp_overlay_init(void* temp, Shared entry); void temp_overlay_destroy(void* temp);
  void entry_copy_construct(void* dst,const void* src); void entry_destroy(void*);
  void create_item(UObject*, Shared, uint8_t); void log_create(Entry*); void log_missing(const Shared*);
  void base_id_add(void* map, const String* id, Shared* entry);
  void group_add(void* groups,const String*,Shared); // folded key; sorted/deduplicated ID
  void* query_created_interface(UObject*); void notify_created(void*);
  void dirty_alloc(void* set, Frame* output); void dirty_construct(void* slot,const String*);
  void dirty_insert(void* set,Frame* result,u32 hash,void* slot); // folded set
  void sparse_grow_slots(Sparse*,i32 oldCount); u64 quantize(u64 bytes,u32 alignment);
  void sparse_grow_bits(Sparse*,i32 oldCount); void sparse_hash_insert(Sparse*,Frame*,u32,void*);
  void sparse_advance(void* iterator); void sparse_destroy_elements(Sparse*,i32);
  void lookup_item(UObject*, Frame* output, const Shared* key); Shared lookup_result(Frame*);
  void* overlay_find_or_add(void* map,const Shared* key,void* temp); // returns value at slot +0x10
  void grow_array(void* header, i32 oldCount); void grow_changes(void* header,i32 oldCount);
  void copy_change(void* dest,const Change*); void copy_stat_change(void* dest,const StatChange*);
  uint8_t apply_fields(void* pair,const void* change);
  void visit_stat_change(const StatChange*, void(*callback)(), void* callback_context);
  void (*stat_callback)();
  void log_count(i32); void finalize_entry(const Shared* key,i32 zero,void* value);
  void collect_group(void* accumulator,Frame* output,const String*);
  void notify_updated(UObject*,Shared,uint16_t flags); void flush_accumulator(UObject*,void*);
  void finish_transaction(void* backing248,i32* transaction,u64 arg4);
};
// Offsets refer to the native image; array headers are data pointer/count/capacity.
static void* at(void* p,size_t n){return static_cast<unsigned char*>(p)+n;}
static const void* at(const void* p,size_t n){return static_cast<const unsigned char*>(p)+n;}
static String* str(void* p,size_t n){return static_cast<String*>(at(p,n));}
static Shared& shared(void* p,size_t n=0){return *static_cast<Shared*>(at(p,n));}
static void dirty(Hooks& h,void* set,const String* key){
  Frame allocation, result; h.dirty_alloc(set,&allocation); void* slot=h.output_slot(&allocation);
  if(slot) h.dirty_construct(slot,key); // allocation failure remains unresolved, as native dereferences below
  auto* s=static_cast<String*>(slot); h.dirty_insert(set,&result,h.hash_string(s),slot);
}
static i32 allocate_sparse(Hooks& h,Sparse& s){
  i32 index=s.firstFree;
  if(s.freeCount==0){
    index=s.count++; if(s.capacity<s.count) h.sparse_grow_slots(&s,index);
    i32 old=s.bitCount;
    if(s.bitCapacity<old+1){
      u32 words=static_cast<u32>((old+0x20+(((old+0x20)>>31)&31))>>5);
      u64 n=words;
      if(words<5) s.bitCapacity=4;
      else {
        i32 previous=(s.bitCapacity+31+(((s.bitCapacity+31)>>31)&31))>>5;
        u64 desired=previous==0 ? (n>1?n:1) : n+(n*3>>3);
        s.bitCapacity=static_cast<i32>(h.quantize(desired*4,0)>>2);
        if(s.bitCapacity<static_cast<i32>(words)) s.bitCapacity=0x7fffffff;
      }
      s.bitCapacity=static_cast<i32>(static_cast<u32>(s.bitCapacity)<<5);
      h.sparse_grow_bits(&s,old);
    }
    ++s.bitCount; u32* bits=s.heapBits?s.heapBits:s.inlineBits;
    bits[(old<0?old+31:old)>>5]&=~(u32(1)<<(old&31));
  } else {
    --s.freeCount; void* slot=at(s.slots,static_cast<ptrdiff_t>(index)*0x18);
    s.firstFree=h.int_at(slot,4);
    if(s.freeCount) h.int_at(at(s.slots,static_cast<ptrdiff_t>(s.firstFree)*0x18),0)=-1;
  }
  u32* bits=s.heapBits?s.heapBits:s.inlineBits;
  bits[(index<0?index+31:index)>>5]|=u32(1)<<(index&31);
  return index;
}
static void touched(Hooks& h,Sparse& s,Shared key){
  i32 index=allocate_sparse(h,s); void* slot=at(s.slots,static_cast<ptrdiff_t>(index)*0x18);
  if(slot){ shared(slot)=key; h.retain(key); h.int_at(slot,0x10)=-1; }
  Frame result; h.sparse_hash_insert(&s,&result,h.hash_string(static_cast<String*>(shared(slot).object)),slot);
}
// Append the transaction group only if the last group has a different signed ID.
static void* append_group(Hooks& h,void* header,i32 id){
  i32 n=h.int_at(header,8); void* data=h.ptr(header,0);
  if(!n || h.int_at(at(data,static_cast<ptrdiff_t>(n-1)*0x18),0)!=id){
    h.int_at(header,8)=n+1; if(h.int_at(header,12)<n+1) h.grow_array(header,n);
    data=h.ptr(header,0); void* g=at(data,static_cast<ptrdiff_t>(n)*0x18);
    h.int_at(g,0)=id; *static_cast<u64*>(at(g,8))=0; *static_cast<u64*>(at(g,16))=0;
  }
  return at(h.ptr(header,0),static_cast<ptrdiff_t>(h.int_at(header,8)-1)*0x18);
}
static void append_change(Hooks& h,void* group,const void* change,bool stats){
  void* array=at(group,8); i32 n=h.int_at(array,8); h.int_at(array,8)=n+1;
  if(h.int_at(array,12)<n+1) h.grow_changes(array,n);
  void* dest=at(h.ptr(array,0),static_cast<ptrdiff_t>(n)*(stats?0xf8:0x110));
  if(stats) h.copy_stat_change(dest,static_cast<const StatChange*>(change));
  else h.copy_change(dest,static_cast<const Change*>(change));
}
void body_1410f2740(Hooks& h,void* backing,i32 transaction,void* batches,u64 arg4,i32& globalSequence){
  UObject* profile=h.weak_get(at(backing,0x17c));
  if(!profile || h.profile_state(profile)!=1) return;
  u32 profileValue=h.read32(backing,0xf8); i32 sequence=++globalSequence;
  Sparse keys{}; keys.bitCapacity=0x80; keys.firstFree=-1;
  alignas(16) unsigned char accumulator[0xf0]; h.init_accumulator(accumulator);
  // The native adjacent output frame has only these fields initialized; do not zero the remainder.
  alignas(16) unsigned char context[0x30];
  *reinterpret_cast<UObject**>(context)=profile; *reinterpret_cast<i32*>(context+8)=sequence;
  *reinterpret_cast<u32*>(context+12)=profileValue; context[16]=0;
  *reinterpret_cast<void**>(context+24)=nullptr; *reinterpret_cast<u64*>(context+32)=0;
  auto* batch=static_cast<unsigned char*>(h.ptr(batches,0));
  auto* end=batch+static_cast<ptrdiff_t>(h.int_at(batches,8))*0x38;
  for(;batch!=end;batch+=0x38){
    auto* created=static_cast<unsigned char*>(h.ptr(batch,8));
    auto* createdEnd=created+static_cast<ptrdiff_t>(h.int_at(batch,0x10))*0xb0;
    for(;created!=createdEnd;created+=0xb0){
      Shared key=h.make_string_copy(reinterpret_cast<String*>(created+0x20)); h.normalize_shared(&key);
      i32 found=h.overlay_find_exact(at(backing,0x198),&key); h.release(key);
      if(found!=-1) continue;
      key=h.make_string_copy(reinterpret_cast<String*>(created+0x20)); h.normalize_shared(&key);
      Shared entry=h.make_entry_copy(created+0x10); h.normalize_shared(&entry);
      alignas(16) unsigned char temp[0xe0]; h.temp_overlay_init(temp,entry);
      Frame allocation,result; h.overlay_alloc(at(backing,0x198),&allocation); void* slot=h.output_slot(&allocation);
      if(slot) h.overlay_construct(slot,key,temp);
      h.overlay_insert(at(backing,0x198),&result,h.hash_string(static_cast<String*>(shared(slot).object)),slot);
      // insertion returns the index in the output frame, not the allocation frame
      i32 index=*reinterpret_cast<i32*>(result.bytes);
      void* overlay=at(h.ptr(backing,0x198),static_cast<ptrdiff_t>(index)*0xe8);
      h.temp_overlay_destroy(temp); h.release(entry); h.release(key);
      h.int_at(overlay,0x24)=transaction;
      if(h.byte_at(overlay,0xc8)){h.byte_at(overlay,0xc8)=0; h.entry_destroy(at(overlay,0x28));}
      h.entry_copy_construct(at(overlay,0x28),created+0x10); h.byte_at(overlay,0xc8)=1; h.byte_at(overlay,0x21)=1;
      Shared value=shared(overlay,0x10); h.retain(value); h.retain(value);
      h.create_item(profile,value,1); h.log_create(static_cast<Entry*>(value.object));
      h.base_id_add(at(backing,8),str(value.object,0x10),&value);
      h.retain(value); h.group_add(at(backing,0x58),str(value.object,0),value);
      h.retain(value); h.group_add(at(backing,0x58),str(value.object,0x20),value);
      if(void* object=h.ptr(value.object,0x88)) if(void* iface=h.query_created_interface(static_cast<UObject*>(object))) h.notify_created(iface);
      Shared touch=h.make_string_copy(str(value.object,0x10)); h.normalize_shared(&touch); touched(h,keys,touch); h.release(touch);
      dirty(h,at(backing,0x2e8),str(value.object,0)); dirty(h,at(backing,0x2e8),str(value.object,0x20)); h.release(value);
    }
    auto* change=static_cast<unsigned char*>(h.ptr(batch,0x18));
    auto* changeEnd=change+static_cast<ptrdiff_t>(h.int_at(batch,0x20))*0x110;
    for(;change!=changeEnd;change+=0x110){
      Shared* key=&shared(change,0xf8); Frame lookup; h.lookup_item(profile,&lookup,key); Shared base=h.lookup_result(&lookup);
      if(!base.object) h.log_missing(key);
      else {
        i32 index=h.overlay_find_exact(at(backing,0x198),key);
        void* overlay=index==-1?nullptr:at(h.ptr(backing,0x198),static_cast<ptrdiff_t>(index)*0xe8);
        void* value=overlay?at(overlay,0x10):nullptr;
        if(!value){
          Shared copy=h.make_entry_copy(base.object); h.normalize_shared(&copy);
          alignas(16) unsigned char temp[0xe0]; h.temp_overlay_init(temp,copy);
          value=h.overlay_find_or_add(at(backing,0x198),key,temp); h.temp_overlay_destroy(temp); h.release(copy);
          dirty(h,at(backing,0x2e8),str(shared(value).object,0));
          dirty(h,at(backing,0x2e8),str(shared(value).object,0x20));
        }
        void* group=append_group(h,at(value,0xc0),transaction); append_change(h,group,change,false);
        void* target=shared(value).object;
        if(h.byte_at(change,0x10c)){
          i32 mode=h.int_at(change,0), amount=h.int_at(change,0x108); i32& quantity=h.int_at(target,0x30);
          if(mode==0) quantity+=amount; else if(mode==1) quantity=amount;
          else if(mode==2 && amount>quantity) quantity=amount;
          h.byte_at(value,0x10)=1;
        }
        void* pair[2]={at(target,0x38),nullptr}; if(h.apply_fields(pair,change)) h.byte_at(value,0x11)=1;
        touched(h,keys,*key);
      }
      h.release(base);
    }
    auto* stat=static_cast<unsigned char*>(h.ptr(batch,0x28));
    auto* statEnd=stat+static_cast<ptrdiff_t>(h.int_at(batch,0x30))*0xf8;
    for(;stat!=statEnd;stat+=0xf8){
      void* group=append_group(h,at(backing,0x1e8),transaction); append_change(h,group,stat,true);
      void* pair[2]={at(backing,0xa8),at(backing,0x1f8)}; h.apply_fields(pair,stat);
      h.visit_stat_change(reinterpret_cast<StatChange*>(stat),h.stat_callback,accumulator);
    }
  }
  h.log_count(keys.count-keys.freeCount);
  // Native sparse iterator visits allocated bits in ascending index order.
  u32* bits=keys.heapBits?keys.heapBits:keys.inlineBits;
  for(i32 i=0;i<keys.bitCount;++i) if(bits[i>>5]&(u32(1)<<(i&31))){
    Shared* key=&shared(at(keys.slots,static_cast<ptrdiff_t>(i)*0x18));
    i32 index=h.overlay_find_exact(at(backing,0x198),key);
    void* overlay=index==-1?nullptr:at(h.ptr(backing,0x198),static_cast<ptrdiff_t>(index)*0xe8);
    void* value=at(overlay,0x10); h.finalize_entry(key,0,value);
    void* target=shared(value).object; Frame first,second;
    h.collect_group(at(accumulator,0x50),&first,str(target,0)); h.collect_group(at(accumulator,0x50),&second,str(target,0x20));
    Shared owned=shared(value); h.retain(owned); h.notify_updated(profile,owned,*static_cast<uint16_t*>(at(overlay,0x20)));
  }
  h.flush_accumulator(profile,accumulator); h.finish_transaction(at(backing,0x248),&transaction,arg4);
  if(*reinterpret_cast<void**>(context+24)) h.free_mem(*reinterpret_cast<void**>(context+24));
  h.destroy_accumulator(accumulator); if(keys.hash) h.free_mem(keys.hash);
  h.sparse_destroy_elements(&keys,0); if(keys.heapBits) h.free_mem(keys.heapBits); if(keys.slots) h.free_mem(keys.slots);
}
}
