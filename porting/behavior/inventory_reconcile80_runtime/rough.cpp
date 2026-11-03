// Source lift: FUN_14201f180. This is a behavioral draft, not a compilable ABI.
// Offsets and numbered calls denote unresolved native layout/virtual contracts.
#include <cstdint>
#include <cstring>
using u8 = uint8_t; using u16 = uint16_t; using u32 = uint32_t; using u64 = uint64_t;
struct Guid { u32 a,b,c,d; };
struct Array { void* data; int32_t num, max; };
struct Shared { void* object; void* controller; };
struct Text { void* data; int32_t num,max; };
struct GroupKey { Text text; u64 v10,v18; void* controller; };
struct Group { Text text; u64 v10,v18; void* controller; int32_t count; };
template<class T> T& at(void* p, size_t n) { return *reinterpret_cast<T*>((u8*)p+n); }
extern void map_empty_142005440(void*,int), map_insert_entry_141ff6ae0(void*,Guid*,void**);
extern void map_insert_slot_141ff6a30(void*,Guid*,void**), map_remove_14201a0d0(void*,Guid*);
extern Guid* entry_guid_14202eda0(void*,Guid*), *item_guid_v468(void*,Guid*);
extern void remove_entry_14201a570(Array*,int,int,int), shrink_14072c6c0(Array*);
// Original decompile omits detach arguments; item is call-site context, not proven ABI.
extern void detach_142066bc0_inferred_item_context(void*);
extern void grow8(Array*,int), resize8_1406d9a30(Array*,int);
extern void* lookup_interface_v18(void*,void* /* unresolved output/arguments */);
extern void* create_14205f090(void*,void*);
extern void log_item_name_v3a8_unresolved_abi(void*), format_142c40540_unresolved_abi();
extern void log_dirty_conflict_fixed_string_unresolved_varargs();
extern int atomic_fetch_sub32(int*,int);
extern int dirty_log_verbosity();
extern void copy_1406af250(void*,void*), copy_1406af1d0(void*,void*), copy_140683960(void*,void*);
extern void copy_142025370(void*,void*), resize_140690a50(void*,int), copy_1409a9160(void*,void*);
extern void text_copy_140682ce0(void*,void*), assign_142ebd9e0(void*,void*);
extern void count_changed_v4e8(void*,int);
struct MapAllocationFrame { u32 meta[2]; void* node; };
extern void map_allocate_140c70c90(void*,MapAllocationFrame*);
extern void map_insert_1420050d0(void*,void*,u32,void*,u64,int);
extern u32 unresolved_incoming_stack_high32();
extern u64 native_dirty_format_literal_address();
extern void resize_append_1406d9a30_unresolved_abi(Array*);
extern u32 hash_142c9fd40(const void*,int);
extern bool authority_test_141ed8de0(void*), item_groupable_v448(void*);
extern void* item_group_key_v450(void*), *allocate_141384ca0(size_t);
extern int compare_142bcf780(const void*,const void*);
extern const void* empty_string_1459a4b30;
extern void group_array_resize_1406911b0(Array*,int), group_array_grow_140690dd0(Array*,int);
extern void controller_destroy_v0(void*), controller_delete_v8(void*,int), free_memory(void*);
extern void initialize_shared_140686df0(Shared*,void*,void*);
extern void* vtable_14605bf90;
extern void sort_141ff8e70(void**,int,int,void*);

static bool zero(Guid g) { return !(g.a|g.b|g.c|g.d); }
static bool equal(Guid x, Guid y) { return x.a==y.a && x.b==y.b && x.c==y.c && x.d==y.d; }
static void append(Array& a, void* p) { int i=a.num++; if(a.max<a.num) grow8(&a,i); ((void**)a.data)[i]=p; }
// Non-atomic shared-reference release as observed in the grouping path.
static void release(void* c) {
  if(c && --at<int32_t>(c,8)==0) { controller_destroy_v0(c); if(--at<int32_t>(c,12)==0) controller_delete_v8(c,1); }
}
static void retain(void* c) { if(c) ++at<int32_t>(c,8); }
static void copy_entry(void* d, void* s) { // 142025400, expanded in native order
#define COPY(T,O) at<T>(d,O)=at<T>(s,O)
  COPY(u32,0xc); COPY(u32,0x10); COPY(u64,0x18); COPY(u16,0x20);
  COPY(u32,0x24); COPY(u32,0x28); COPY(u32,0x2c);
  copy_1406af250((u8*)d+0x30,(u8*)s+0x30); copy_1406af1d0((u8*)d+0xb8,(u8*)s+0xb8);
  copy_140683960((u8*)d+0x40,(u8*)s+0x40);
  u32 v54=at<u32>(s,0x54), v58=at<u32>(s,0x58), v5c=at<u32>(s,0x5c);
  COPY(u32,0x50); at<u32>(d,0x54)=v54; at<u32>(d,0x58)=v58; at<u32>(d,0x5c)=v5c; COPY(u32,0x60); COPY(u8,0x64);
  copy_142025370((u8*)d+0x70,(u8*)s+0x70); COPY(u8,0x65);
  auto& da=at<Array>(d,0x98); auto& sa=at<Array>(s,0x98);
  if(&da!=&sa) { int n=sa.num; void* data=sa.data; da.num=n;
    if(n==0 && da.max==0) da.max=0;
    else { resize_140690a50(&da,n); std::memcpy(da.data,data,(int64_t)n*16); }
  }
  COPY(u32,0xb0); copy_1409a9160((u8*)d+0xf0,(u8*)s+0xf0); COPY(u8,0x68);
  COPY(u64,0xc8); COPY(u32,0xd0);
  struct { u64 first; u8 text[24]; } temp; temp.first=at<u64>(s,0xd8);
  text_copy_140682ce0(&temp.text,(u8*)s+0xe0); assign_142ebd9e0((u8*)d+0xd8,&temp);
#undef COPY
}
// Hash bucket walk is explicit: the map value is an address, possibly invalidated later.
static void** find_entry(void* inv, Guid g) {
  if(at<int>(inv,0x348)==at<int>(inv,0x374)) return nullptr;
  u32 h=hash_142c9fd40(&g,16); auto buckets=at<int*>(inv,0x380);
  if(!buckets) buckets=(int*)((u8*)inv+0x378);
  int i=buckets[(at<int>(inv,0x388)-1u)&h];
  while(i!=-1) { auto p=(u8*)at<void*>(inv,0x340)+(int64_t)i*0x20;
    if(equal(at<Guid>(p,0),g)) return (void**)(p+0x10); i=at<int>(p,0x18); }
  return nullptr;
}
static void append_all(Array& dst, Array& src) {
  if(src.num) { int n=src.num, old=dst.num; if(dst.max<old+n) { resize_append_1406d9a30_unresolved_abi(&dst); old=dst.num; }
    std::memcpy((void**)(dst.data)+old,src.data,(int64_t)n*8); dst.num+=n; }
}
void reconcile_14201f180(void* inv) {
  auto& entries=at<Array>(inv,0x330); auto& items=at<Array>(inv,0x390);
  void* entrymap=(u8*)inv+0x340; void* itemmap=(u8*)inv+0x3a0;
  map_empty_142005440(entrymap,0);
  for(int i=0;i<entries.num;++i) { void* entry=(u8*)entries.data+(int64_t)i*0x150; Guid g; auto key=entry_guid_14202eda0(entry,&g); map_insert_entry_141ff6ae0(entrymap,key,&entry); }
  for(int i=0;i<items.num;++i) if(!((void**)items.data)[i]) { remove_entry_14201a570(&entries,i,1,1); --i; }
  map_empty_142005440(itemmap,0);
  for(int i=0;i<items.num;++i) { void** slot=(void**)items.data+i; Guid g; auto key=item_guid_v468(*slot,&g); map_insert_slot_141ff6a30(itemmap,key,slot); }
  Array removed{}, added{}, modified{};
  for(int i=0;i<items.num;) {
    void* item=((void**)items.data)[i]; Guid g; item_guid_v468(item,&g);
    void** found=zero(g)?nullptr:find_entry(inv,g);
    if(zero(g) || !found || !*found) {
      append(removed,item); int n=items.num; int count=n-i-1; if(count>0) count=1;
      if(count) std::memcpy((void**)items.data+i,(void**)items.data+n-count,(int64_t)count*8);
      --items.num; shrink_14072c6c0(&items); map_remove_14201a0d0(itemmap,&g); detach_142066bc0_inferred_item_context(item);
    } else ++i;
  }
  u64 stack_word=u64(unresolved_incoming_stack_high32())<<32; // low word overwritten before use
  struct LookupFrame { u8 scratch[24]; void* controller; } lookup_frame;
  MapAllocationFrame allocation_frame; // original stack slots reused, no pre-zeroing
  for(int i=0;i<entries.num;++i) {
    void* entry=(u8*)entries.data+(int64_t)i*0x150; Guid g; entry_guid_14202eda0(entry,&g);
    void* item=lookup_interface_v18((u8*)inv+0x218,&lookup_frame);
    if(!item) {
      at<u8>(entry,0x67)=0; item=nullptr; auto& pending=at<Array>(inv,0x410);
      for(int j=0;j<pending.num;++j) { void* candidate=((void**)pending.data)[j]; if(candidate) { Guid tmp; Guid* key=item_guid_v468(candidate,&tmp); if(equal(g,*key)) item=candidate; } }
      if(!item) item=create_14205f090(inv,entry);
      append(added,item); append(items,item);
      map_allocate_140c70c90(itemmap,&allocation_frame);
      void* node=allocation_frame.node;
      if(node) { at<void*>(node,0x10)=item; at<Guid>(node,0)=g; at<u32>(node,0x18)=0xffffffff; }
      u32 h=hash_142c9fd40(node,16); u8 insert_result[8];
      stack_word=(stack_word & 0xffffffff00000000ULL)|allocation_frame.meta[0];
      map_insert_1420050d0(itemmap,insert_result,h,node,stack_word,0);
    } else if(at<u8>(entry,0x67)) {
      if(at<u8>(item,0x147) && dirty_log_verbosity()>1) {
        log_item_name_v3a8_unresolved_abi(item); format_142c40540_unresolved_abi();
        stack_word=native_dirty_format_literal_address();
        log_dirty_conflict_fixed_string_unresolved_varargs();
        void* c=lookup_frame.controller;
        if(c && atomic_fetch_sub32(&at<int>(c,8),1)==1) {
          controller_destroy_v0(c);
          if(atomic_fetch_sub32(&at<int>(c,12),1)==1) controller_delete_v8(c,1);
        }
      }
      int old=at<int>(item,0xec); at<u8>(entry,0x67)=0; copy_entry((u8*)item+0xe0,entry);
      if(old!=at<int>(entry,0xc)) count_changed_v4e8(item,old); append(modified,item);
    } else if(at<u8>(item,0x147)) { at<u8>(item,0x147)=0; append(modified,item); }
  }
  bool sort=authority_test_141ed8de0(inv) && entries.num>=2 && (removed.num>0 || added.num>0);
  auto& groups=at<Array>(inv,0x400);
  for(int i=0;i<groups.num;++i) release(((Shared*)groups.data)[i].controller);
  groups.num=0; if(groups.max) group_array_resize_1406911b0(&groups,0);
  for(int i=0;i<items.num;++i) {
    void* item=((void**)items.data)[i]; if(!item || !item_groupable_v448(item)) continue;
    void* source=item_group_key_v450(item); GroupKey key;
    text_copy_140682ce0(&key.text,source); key.v10=at<u64>(source,0x10); key.v18=at<u64>(source,0x18); key.controller=at<void*>(source,0x20); retain(key.controller);
    int match=-1;
    for(int j=0;j<groups.num;++j) { Group* group=(Group*)((Shared*)groups.data)[j].object;
      const void* a=group->text.num?group->text.data:empty_string_1459a4b30;
      const void* b=key.text.num?key.text.data:empty_string_1459a4b30;
      if(compare_142bcf780(a,b)==0) { match=j; break; }
    }
    if(match>=0) ++((Group*)((Shared*)groups.data)[match].object)->count;
    else {
      GroupKey temp; text_copy_140682ce0(&temp.text,&key.text); temp.v10=key.v10; temp.v18=key.v18; temp.controller=key.controller; retain(temp.controller);
      Group* group=(Group*)allocate_141384ca0(0x30);
      if(group) { GroupKey arg; text_copy_140682ce0(&arg.text,&temp.text); arg.v10=temp.v10; arg.v18=temp.v18; arg.controller=temp.controller; retain(arg.controller);
        text_copy_140682ce0(&group->text,&arg.text); group->v10=arg.v10; group->v18=arg.v18; group->controller=arg.controller; retain(group->controller); group->count=1;
        release(arg.controller); if(arg.text.data) free_memory(arg.text.data);
      }
      void* controller=allocate_141384ca0(0x18); if(controller) { at<u32>(controller,8)=1; at<void*>(controller,0)=vtable_14605bf90;
        at<u32>(controller,12)=1; at<void*>(controller,16)=group; }
      Shared shared{group,controller}; initialize_shared_140686df0(&shared,group,group);
      release(temp.controller); if(temp.text.data) free_memory(temp.text.data);
      int j=groups.num++; if(groups.max<groups.num) group_array_grow_140690dd0(&groups,j);
      ((Shared*)groups.data)[j]=shared; retain(shared.controller); release(shared.controller);
    }
    release(key.controller); if(key.text.data) free_memory(key.text.data);
  }
  if(sort && at<u8>(inv,0xf0)!=3) { bool all=true; for(int i=0;i<items.num;++i) if(!((void**)items.data)[i]) { all=false; break; }
    if(all) { u64 context=(uintptr_t)inv & ~u64(0xff); sort_141ff8e70((void**)items.data,items.num,0,&context); }
  }
  auto& pending=at<Array>(inv,0x410); pending.num=0; if(pending.max) resize8_1406d9a30(&pending,0);
  append_all(at<Array>(inv,0x420),added); append_all(at<Array>(inv,0x430),removed); append_all(at<Array>(inv,0x440),modified);
  if(modified.data) free_memory(modified.data); if(added.data) free_memory(added.data); if(removed.data) free_memory(removed.data);
}
