// Uncounted source drafts; not a native build input. All adapters are unresolved.
#include <cstdint>
namespace sky_stream78 {
using U = std::uint64_t; using I = std::int32_t; using B = std::uint8_t;
template<class T> T& at(U p, U o = 0) { return *reinterpret_cast<T*>(p + o); }
template<U Address, class... A> U call(A...);
template<class... A> U virt(U object, unsigned slot, A...);
extern B global_1482472e4();
extern U global_14785c5a0_address();
extern U global_14785c5a0_data();
extern void free_memory(U);
extern void FTimerManager_InternalClearTimer(U manager, U handle);
// Conceptual stack output, not a verified native container layout.
struct Components {
 U inline_items[24];
 U heap_items;
 I count;
 std::uint32_t capacity;
};
void SkyTubeEnableEffects(U self) {
 call<0x1440fc970>(at<U>(self, 0x220));
 U match = 0;
 const wchar_t* stat = L"STAT_GetComponentsTime";
 Components components;
 components.heap_items = 0;
 components.capacity = 0x18;
 if (global_1482472e4()) stat = nullptr;
 components.count = 0;
 U type = call<0x1448013f0>();
 if (type) {
  U base = call<0x1446b3d40>();
  if (base && at<I>(base, 0x38) <= at<I>(type, 0x38) &&
      at<U>(at<U>(type, 0x30), U(at<I>(base, 0x38)) * 8) == base + 0x30)
   match = type;
 }
 U base = call<0x1446b3d40>();
 if (match == base) call<0x1406a90d0>();
 else call<0x1406a92e0>(self, type, reinterpret_cast<U>(&components));
 if (global_1482472e4() && stat)
  call<0x142cd3770>(reinterpret_cast<U>(&stat));
 U items = components.heap_items ? components.heap_items : reinterpret_cast<U>(components.inline_items);
 U end = items + std::int64_t(components.count) * 8;
 for (U p = items; p != end; p += 8) {
  U component = at<U>(p);
  if (component) call<0x1440fc970>(component, at<B>(self, 0x5c0));
 }
 if (components.heap_items) free_memory(components.heap_items);
}
void StreamingHandshakeStart(U self) {
 if (at<B>(self, 0xf0) != 3) {
  U world = virt(self, 0x150); // Source gives no explicit virtual arguments.
  if (world) {
   I index[2];
   call<0x1407bda90>(global_14785c5a0_address(), reinterpret_cast<U>(index), world);
   if (index[0] != -1) {
    U entry = global_14785c5a0_data() + std::int64_t(index[0]) * 0x18;
    if (entry) {
     U level = at<U>(entry, 8);
     if (level) {
      U selector = (U(std::int64_t(index[0]) * 3) & ~U(0xff)) | 1;
      call<0x141d58ea0>(level, selector, L"SetupLevelStreamingOnClient");
      at<B>(level, 0x15a) = 1;
      at<B>(self, 0x5b1 * 8) = 0;
      call<0x142a54b40>(self, 0);
     }
    }
   }
  }
 }
}
void StreamingHandshakeTimer(U self) {
 if (B(call<0x1419dbc20>())) {
  call<0x1419c5210>(self);
  U world = virt(self, 0x150, self);
  U owner = at<U>(world, 0x188);
  U manager = owner ? at<U>(owner, 0xc0) : at<U>(world, 800);
  U handle_address = self + 0x5b2 * 8;
  if (call<0x144621fb0>(manager, handle_address))
   FTimerManager_InternalClearTimer(manager, at<U>(handle_address));
  at<U>(handle_address) = 0;
 }
}
}
