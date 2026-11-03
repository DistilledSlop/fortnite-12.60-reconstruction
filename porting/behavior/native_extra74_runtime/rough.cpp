// Behavioral reconstruction, not a linkable ABI declaration. See HANDOFF.md.
#include <cstdint>
#include <cstddef>
using u8 = std::uint8_t; using u32 = std::uint32_t; using u64 = std::uint64_t;
using Address = std::uintptr_t;
struct RawFrame { alignas(16) u8 bytes[256]; }; // deliberately uninitialized
extern Address at(Address, std::ptrdiff_t); // address arithmetic, including native address values
extern Address slot(RawFrame&, std::size_t);
extern u8 load8(Address); extern u32 load32(Address); extern u64 load64(Address);
extern void store8(Address,u8); extern void store32(Address,u32); extern void store64(Address,u64);
extern void copy_bytes(Address, Address, std::size_t);
extern u32 atomic_fetch_add32(Address, std::int32_t);
extern Address global_1481b0f48(); extern u64 global_1480c4288();
extern Address native_142c0c240(Address); extern void native_14079d060(Address,Address);
extern Address native_142ec7730(Address,Address);
extern void virtual_210(Address vtable, Address self, Address function, Address args);
extern void virtual_0(Address control); extern void virtual_8(Address control,u32 flag);
extern Address native_140f95ae0(); extern Address native_144621fb0(Address,Address);
extern Address native_142bfd200(Address,std::size_t,u32); extern void native_142bee710(Address);
extern void native_142ee3b30(Address,Address); extern u64 native_142bad740();
extern void virtual_40(Address object,Address output); extern void virtual_38(Address object,u32 flag);
extern void virtual_10(Address object);
// Includes the native stack arguments 1 and -1.0f and opaque live stack locals:
extern void native_1446278d0(Address manager, Address handle, Address delegate, float delay,
                              u8 stack_arg5, u32 stack_arg6_bits, RawFrame& live_frame);
extern void native_14193c3d0(Address); extern void native_145803c70(u64);
extern u64 native_stack_cookie_frame_address(RawFrame&); // actual native RSP for cookie protocol

void PrepareRespawnForwardTarget142a53030(Address object, Address first12, Address second12,
                                          u32 scalar_bits, u64 value, Address source) {
  RawFrame f;
  // Coordinates use offsets from native RSP after its prologue.
  store64(slot(f,0x3c),0);
  native_142c0c240(slot(f,0x48));
  // Reads occur in native order, and each 12-byte input is copied without padding.
  u32 first_tail = load32(at(first12,8));
  u64 first_head = load64(first12);
  store32(slot(f,0x28),first_tail);
  u32 second_tail = load32(at(second12,8));
  store32(slot(f,0x34),second_tail);
  store64(slot(f,0x20),first_head);
  u64 second_head = load64(second12);
  store64(slot(f,0x3c),value);
  store32(slot(f,0x38),scalar_bits);
  store64(slot(f,0x2c),second_head);
  native_14079d060(slot(f,0x48),source);
  Address table = load64(object); // captured before FindFunctionChecked
  Address function = native_142ec7730(object,global_1481b0f48());
  virtual_210(table,object,function,slot(f,0x20));
  Address control = load64(slot(f,0x50));
  if (control && atomic_fetch_add32(at(control,8),-1) == 1) {
    virtual_0(control);
    if (atomic_fetch_add32(at(control,12),-1) == 1) virtual_8(control,1);
  }
}

void RemoveAllLootFromWorldForwardTarget14195e100(Address self, u32 a, u32 b,
                                                   float delay, u8 option) {
  RawFrame f;
  // Native offsets below are relative to RBP. Bias 0x80 keeps negative offsets in frame.
  auto rbp = [&](int off) { return slot(f,static_cast<std::size_t>(0x80 + off)); };
  u64 cookie = global_1480c4288() ^ native_stack_cookie_frame_address(f);
  store64(rbp(-0x61),load64(at(self,0x1570)));
  Address manager = native_140f95ae0();
  Address found = native_144621fb0(manager,rbp(-0x61));
  if (!found || load8(at(found,1)) == 2) {
    store64(at(self,0x1560),~u64(0));
    store8(at(self,0x1568),1);
    if (delay > 0.0f) { // unordered comparisons also skip this block
      store8(at(self,0x1568),option);
      store64(rbp(-0x61),0x14193c3d0ULL);
      store32(at(self,0x1560),a); store32(at(self,0x1564),b);
      store32(rbp(-0x59),0); // upper four bytes of the 16-byte source remain native stack contents
      manager = native_140f95ae0();
      Address original = native_142bfd200(0,0x30,0);
      if (original) {
        store64(original,0x1459a9e80ULL);
        native_142ee3b30(at(original,8),self);
        copy_bytes(at(original,0x10),rbp(-0x61),16);
        store64(at(original,0x28),native_142bad740());
        store64(original,0x1459a9ed8ULL);
      }
      store64(rbp(-0x51),0); store32(rbp(-0x49),0);
      if (original) virtual_40(original,rbp(-0x51));
      native_142ee3b30(rbp(-0x41),0);
      store64(rbp(-0x39),0); store64(rbp(-0x31),0); store64(rbp(-0x21),0);
      native_1446278d0(manager,at(self,0x1570),rbp(-0x51),delay,1,0xbf800000,f);
      if (load64(rbp(-0x31))) {
        Address cleanup = load64(rbp(-0x21));
        virtual_10(cleanup ? cleanup : rbp(-0x11));
      }
      u32 count = load32(rbp(-0x49));
      Address pointer = load64(rbp(-0x51));
      Address selected = count ? pointer : 0;
      if (selected || (count && pointer)) {
        virtual_38(selected ? selected : pointer,0);
        pointer = load64(rbp(-0x51)); // destructor may mutate the live local
        if (pointer) { pointer = native_142bfd200(pointer,0,0); store64(rbp(-0x51),pointer); }
        store32(rbp(-0x49),0);
        if (pointer) native_142bee710(pointer);
      } else if (!count) {
        if (pointer) native_142bee710(pointer);
      }
      if (original) {
        virtual_38(original,0);
        Address released = native_142bfd200(original,0,0);
        if (released) native_142bee710(released);
      }
    }
    native_14193c3d0(self);
  }
  native_145803c70(cookie ^ native_stack_cookie_frame_address(f));
}
