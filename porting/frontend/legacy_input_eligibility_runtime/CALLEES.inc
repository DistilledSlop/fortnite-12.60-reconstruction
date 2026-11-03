// Bodies for resolved callees. Source of truth: CALLEES.md in this folder.
// Not built or run. Globals are addressed by VA and are not named.
// Layouts below are verified only where CALLEES.md says so.

// ---- 142ee5dd0: FWeakObjectPtr::Get() (bEvenIfPendingKill = false) ----
// Asm/decompile: serial==0 -> null; index<0 or index>=count -> null; chunk = index>>16,
// item = chunk[index&0xffff] (stride 0x18); item null or serial mismatch -> null;
// flags & (Unreachable|PendingKill)=0x30000000 -> null; else item->Object.
struct NativeFUObjectItem142ee5dd0 { void* Object; uint32 Flags; int32 ClusterRootIndex; int32 SerialNumber; };
extern int32 GNativeUObjectCount142ee5dd0;                 // global 0x148263a2c
extern NativeFUObjectItem142ee5dd0** GNativeUObjectChunks142ee5dd0; // global 0x148263a18 (chunk table)

inline void* NativeWeakGet142ee5dd0(int32 Index, int32 Serial)
{
    if (Serial == 0 || Index < 0 || Index >= GNativeUObjectCount142ee5dd0) return nullptr;
    NativeFUObjectItem142ee5dd0* Chunk = GNativeUObjectChunks142ee5dd0[Index >> 16];
    NativeFUObjectItem142ee5dd0* Item  = (NativeFUObjectItem142ee5dd0*)((uint8*)Chunk + (uint32)(Index & 0xffff) * 0x18);
    if (!Item || Item->SerialNumber != Serial || (Item->Flags & 0x30000000u) != 0) return nullptr;
    return Item->Object;
}

// ---- 142ee7880: FWeakObjectPtr::IsValid() ----
// Same checks as 142ee5dd0 but returns bool and does not read Object.
inline bool NativeWeakIsValid142ee7880(int32 Index, int32 Serial)
{
    if (Serial == 0 || Index < 0 || Index >= GNativeUObjectCount142ee5dd0) return false;
    NativeFUObjectItem142ee5dd0* Chunk = GNativeUObjectChunks142ee5dd0[Index >> 16];
    NativeFUObjectItem142ee5dd0* Item  = (NativeFUObjectItem142ee5dd0*)((uint8*)Chunk + (uint32)(Index & 0xffff) * 0x18);
    return Item && Item->SerialNumber == Serial && (Item->Flags & 0x30000000u) == 0;
}

// ---- 142ee3b30: FWeakObjectPtr(const UObject*) / operator= ----
// Asm 2ee3b30-2ee3b6a. Out[0] = ObjectIndex, Out[1] = ObjectSerialNumber.
// Object+0xc is UObjectBase::InternalIndex. The serial call takes this=0x148263810, index in edx.
extern int32 NativeAllocateSerialNumber142ec00d0(void* GUObjectArrayThis, int32 Index); // callee, identity not verified
extern void* GNativeGUObjectArray142ee3b30;                 // VA 0x148263810
inline void NativeWeakFromObject142ee3b30(int32 Out[2], const uint8* Object)
{
    if (!Object) { Out[0] = -1; Out[1] = 0; return; }        // Reset()
    Out[0] = *(const int32*)(Object + 0xc);                  // InternalIndex
    Out[1] = NativeAllocateSerialNumber142ec00d0(GNativeGUObjectArray142ee3b30, Out[0]);
}

// ---- 143d5a140: project-local NativeSharedAccessor143d5a140 (NOT UWidget::GetCachedWidget) ----
// sret {Object, Ctrl}. Controller: vtbl +0 (DestroyObject), vtbl +8 (deleting dtor),
// SharedRefCount +8, WeakRefCount +0xc (FReferenceControllerBase).
// Strong member at self+0xe8/+0xf0 is copied when object, ctrl and SharedRefCount>=1 hold.
// Otherwise weak member at self+0xd8/+0xe0 is pinned: if its SharedRefCount is 0, result is null.
struct NativeRefCtrl143d5a140 { void* Vtbl; int32 SharedRefCount; int32 WeakRefCount; };
struct NativeSharedOut143d5a140 { void* Object; NativeRefCtrl143d5a140* Ctrl; };
inline NativeSharedOut143d5a140 NativeSharedAccessor143d5a140(const uint8* Self)
{
    NativeSharedOut143d5a140 Out{nullptr, nullptr};
    void* SObj = *(void* const*)(Self + 0xe8);
    NativeRefCtrl143d5a140* SCtrl = *(NativeRefCtrl143d5a140* const*)(Self + 0xf0);
    if (SObj != nullptr && SCtrl != nullptr && SCtrl->SharedRefCount >= 1)
    {
        SCtrl->SharedRefCount += 1;                          // net +1 (binary: +1, +1, -1)
        Out.Object = SObj;
        Out.Ctrl = SCtrl;
        return Out;
    }
    NativeRefCtrl143d5a140* WCtrl = *(NativeRefCtrl143d5a140* const*)(Self + 0xe0);
    if (WCtrl != nullptr && WCtrl->SharedRefCount != 0)
    {
        WCtrl->SharedRefCount += 1;
        Out.Object = *(void* const*)(Self + 0xd8);
        Out.Ctrl = WCtrl;
    }
    return Out;
}

// ---- 142bee710: project-local NativeGMallocSlot30142bee710 (identity NOT established) ----
// if (p) { if (G == 0) FUN_142bee820(); G->vtbl[0x30](G, p); }   G = *(void**)0x14823d250.
// In the 4.25 FMalloc order slot 0x30 is TryRealloc, not Free, so no name is claimed.
extern void* GNativeAllocator142bee710;                     // VA 0x14823d250
extern void NativeAllocatorInit142bee820();                 // FUN_142bee820
inline void NativeGMallocSlot30142bee710(void* P)
{
    if (!P) return;
    if (GNativeAllocator142bee710 == nullptr) NativeAllocatorInit142bee820();
    using Fn = void (*)(void*, void*);
    (*(Fn*)((*(void***)GNativeAllocator142bee710)[6]))(GNativeAllocator142bee710, P);
}

// ---- 142bfd200: project-local NativeGMallocSlot20142bfd200 (identity NOT established) ----
// if (G == 0) FUN_142bee820(); G->vtbl[0x20](G, a, b, c).
inline void NativeGMallocSlot20142bfd200(void* A, void* B, uint32 C)
{
    if (GNativeAllocator142bee710 == nullptr) NativeAllocatorInit142bee820();
    using Fn = void (*)(void*, void*, void*, uint32);
    (*(Fn*)((*(void***)GNativeAllocator142bee710)[4]))(GNativeAllocator142bee710, A, B, C);
}
