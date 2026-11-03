// Uncounted behavioral drafts. These declarations are unresolved native ABI contracts.
#include <cstdint>
#include <cstring>
namespace pickup_spawn77 {
using u8 = uint8_t; using u16 = uint16_t; using u32 = uint32_t;
using i32 = int32_t; using u64 = uint64_t; using byte = unsigned char;
template<class T> static T rd(const void* p, intptr_t n) { T v; std::memcpy(&v, static_cast<const byte*>(p)+n, sizeof v); return v; }
template<class T> static void wr(void* p, intptr_t n, T v) { std::memcpy(static_cast<byte*>(p)+n, &v, sizeof v); }
static void* add(void* p, intptr_t n) { return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p)+n); }
extern u8 dat_1482472e4(); extern void fun_142cd3770(const wchar_t**);
extern char thunk_141ed8de0(); extern char thunk_141ed8de0_with_pickup(void*);
extern void* fun_142a29210(); extern void* fun_142a292d0();
extern void fun_144680860(void*); extern void class_virtual_390(void*);
extern u64 dat_148218630(); extern float dat_148218638();
extern void* fun_1442e74f0(void*, void*, void*, void*, void*);
extern int16_t fun_1423881c0(); extern void pickup_virtual_6c8(void*);
struct Array { void* data; i32 num; i32 max; };
extern void pickup_virtual_6b0(void*, void*, Array*, int);
extern void fmemory_free(void*); extern void fun_1410f8b30(void*, int);
extern void ref_virtual_0(void*); extern void ref_virtual_8(void*);

// Source class test: the candidate is a class-like pointer with ancestor table at +30.
static bool class_contains(void* candidate, void* base) {
  i32 depth = rd<i32>(base, 0x38);
  return depth <= rd<i32>(candidate, 0x38) &&
    rd<void*>(rd<void*>(candidate, 0x30), intptr_t(depth)*8) == add(base, 0x30);
}
static void free_if(void* p) { if (p) fmemory_free(p); }
static void cleanup_results(Array& a) {
  if (a.num != 0) {
    byte* cursor = static_cast<byte*>(a.data) + 0x140;
    i32 remaining = a.num;
    do {
      wr<u32>(cursor, 8, 0);
      free_if(rd<void*>(cursor, 0));
      fun_1410f8b30(cursor - 0x40, 0);
      free_if(rd<void*>(cursor, -0x20));
      free_if(rd<void*>(cursor, -0x40));
      free_if(rd<void*>(cursor, -0x50));
      free_if(rd<void*>(cursor, -0x60));
      free_if(rd<void*>(cursor, -0x88));
      free_if(rd<void*>(cursor, -0xa8));
      void* ref = rd<void*>(cursor, -0xb0);
      if (ref) {
        i32 strong = rd<i32>(ref, 8) - 1; wr<i32>(ref, 8, strong);
        if (strong == 0) {
          ref_virtual_0(ref);
          i32 weak = rd<i32>(ref, 0xc) - 1; wr<i32>(ref, 0xc, weak);
          if (weak == 0) ref_virtual_8(ref);
        }
      }
      free_if(rd<void*>(cursor, -0xd0));
      free_if(rd<void*>(cursor, -0x100));
      i32 count = rd<i32>(cursor, -0x108);
      void* entries = rd<void*>(cursor, -0x110);
      if (count != 0) {
        byte* entry = static_cast<byte*>(entries);
        do { free_if(rd<void*>(entry, 0)); entry += 16; --count; } while (count != 0);
        entries = rd<void*>(cursor, -0x110);
      }
      free_if(entries);
      cursor += 0x150; --remaining;
    } while (remaining != 0);
  }
  free_if(a.data);
}
void* pickup_spawn_from_parameters(void* params) {
  const wchar_t* stat = L"STAT_CreatePickupFromData";
  void* pickup = nullptr;
  if (dat_1482472e4() != 0) stat = nullptr;
  if (rd<void*>(params, 0x20) != nullptr && rd<i32>(params, 0x14) >= 1) {
    pickup = thunk_141ed8de0() == 0 ? fun_142a29210() : fun_142a292d0();
    void* override_class = rd<void*>(params, 0x178);
    if (override_class) {
      void* base = fun_142a29210();
      if (base && class_contains(override_class, base) && override_class) {
        base = fun_142a29210();
        if (base) {
          pickup = rd<void*>(params, 0x178);
          if (!class_contains(pickup, base)) pickup = nullptr;
        } else pickup = nullptr;
      }
    }
    // Native owner stack48 follows constructor storage stack58 by16 bytes.
    struct SpawnOptions { byte prefix[16]; void* owner; } spawn_options;
    fun_144680860(&spawn_options);
    void* owner = rd<void*>(params, 0x170);
    if (!owner) owner = rd<void*>(params, 0x180);
    spawn_options.owner = owner;
    // Separate stack locals: constructor output and owner may be ABI-related.
    struct Location { u64 xy; float z; } location;
    location.xy = rd<u64>(params, 0x158); location.z = rd<float>(params, 0x160);
    if (rd<void*>(params, 0x180)) {
      void* component = rd<void*>(pickup, 0x130);
      if (!component) { class_virtual_390(pickup); component = rd<void*>(pickup, 0x130); }
      if (component) {
        void* bounds = rd<void*>(component, 0x478);
        if (bounds) {
          void* root = rd<void*>(rd<void*>(params, 0x180), 0x130);
          u64 xy; float z;
          if (!root) { xy = dat_148218630(); z = dat_148218638(); }
          else { xy = rd<u64>(root, 0x1d0); z = rd<float>(root, 0x1d8); }
          // The source copies xy/z to a second local; the spawn-call alias is unresolved.
          u64 copied_xy = xy; float copied_z = z; (void)copied_xy; (void)copied_z;
          if (z <= location.z) {
            float adjusted = rd<float>(bounds, 0x1e8) * rd<float>(bounds, 0x400) + z + 0.0001f;
            if (location.z < adjusted) location.z = adjusted;
          }
        }
      }
    }
    pickup = fun_1442e74f0(rd<void*>(params, 0), pickup, &location, add(params, 0x164), &spawn_options);
    if (pickup) {
      u8 flags = rd<u8>(params, 0x18a);
      if (rd<u8>(pickup, 0xf0) == 3) {
        int16_t id;
        bool assign = true;
        if (!rd<void*>(params, 0x170)) id = -1;
        else { id = fun_1423881c0(); if (rd<u8>(pickup, 0xf0) != 3) assign = false; }
        if (assign) {
          wr<int16_t>(pickup, 0x452, id);
          if (id != -1 && thunk_141ed8de0_with_pickup(pickup) == 0) {
            wr<u8>(pickup, 0x58, rd<u8>(pickup, 0x58) & 0xf7);
            wr<u8>(pickup, 0x58, rd<u8>(pickup, 0x58) | ((flags >> 1 & 1) << 3));
          }
          pickup_virtual_6c8(pickup);
        }
      }
      Array results{nullptr, 0, 0};
      pickup_virtual_6b0(pickup, add(params, 8), &results, 0);
      cleanup_results(results);
      wr<u8>(pickup, 0x288, rd<u8>(params, 0x18a) & 1);
      wr<u8>(pickup, 0x450, rd<u8>(pickup, 0x450) | rd<u8>(params, 0x188));
      wr<u8>(pickup, 0x451, rd<u8>(params, 0x189));
    }
  }
  if (dat_1482472e4() != 0 && stat != nullptr) fun_142cd3770(&stat);
  return pickup;
}

extern void* fun_141c74820(void*); extern void* fun_141c7cde0(void*, void*);
extern void* afort_player_state_athena_private_static_class();
// The decompiled UObject[1] member offsets require actual UObject layout.
extern i32 player_state_static_class_depth(void*); extern void* player_state_static_class_ancestor_address(void*);
extern char permission_interface_virtual_28(void*);
extern u64 permission_interface_virtual_48(void* dispatch_interface, u64, void*, void*, void*);
struct Position { u64 first; u32 last; };
u64 creative_spawn_permission(void* position, void* item, void* controller, void* fallback) {
  if (item) {
    void* world = fun_141c74820(controller ? controller : fallback);
    if (world) {
      Position copy; copy.last = rd<u32>(position, 8); copy.first = rd<u64>(position, 0);
      void* region = fun_141c7cde0(world, &copy);
      if (region && controller) {
        void* state = rd<void*>(controller, 0x220);
        if (state) {
          void* type = afort_player_state_athena_private_static_class();
          void* state_class = rd<void*>(state, 0x10);
          i32 depth = player_state_static_class_depth(type);
          if (depth <= rd<i32>(state_class, 0x38) &&
              rd<void*>(rd<void*>(state_class, 0x30), intptr_t(depth)*8) == player_state_static_class_ancestor_address(type)) {
            copy.first = rd<u64>(position, 0);
            u32 last = rd<u32>(position, 8);
            bool enabled = false;
            if (rd<i32>(region, 0x468) != 0) {
              void* iface = rd<void*>(region, 0x460);
              if (iface && permission_interface_virtual_28(iface) != 0) enabled = true;
            }
            if (enabled) {
              void* iface = rd<i32>(region, 0x468) == 0 ? nullptr : rd<void*>(region, 0x460);
              copy.last = last;
              // Native decompiler dispatches via iface but omits it from visible arguments.
              return permission_interface_virtual_48(iface, copy.first, &copy, item, add(state, 0x1228));
            }
          }
        }
      }
    }
  }
  return 1;
}
}
