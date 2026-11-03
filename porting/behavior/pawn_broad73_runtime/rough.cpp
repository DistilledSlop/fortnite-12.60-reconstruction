// Uncounted behavioral drafts, not build inputs. All calls and storage names
// describe observed boundaries, not resolved native signatures or UE types.
#include <cstdint>
#include <cstring>
namespace pawn_broad73 {
using U8 = std::uint8_t; using U32 = std::uint32_t; using U64 = std::uint64_t;
struct Pawn; struct Frame;
template<class T> T& at(void* p, unsigned n) { return *reinterpret_cast<T*>(static_cast<U8*>(p)+n); }
extern void Finish(Frame*);
extern void Step(Frame*, void*);
extern void* GliderStack(Pawn*); extern void SetGliderClassNull(Pawn*);
extern bool Virtual6b0(Pawn*); extern void Virtual1208(Pawn*);
extern void* FUN_141df62a0_UnresolvedArgs();
extern void* ElementVirtual400(void*, void* scratch);
extern void Grow16_140690dd0(void* array, U32 oldCount);
extern void Grow8_1406d93e0(void* array, int count);
extern U64 Global_14812b6e0();
extern void ServiceVirtual330(void* service, void* scratch, void* names,
                              void* filter, void* output, int zero1,
                              void* pair, int zero2);
extern void Free(void*); extern void ControlVirtual0(void*); extern void ControlVirtual8(void*, int);
void Release(void* control) {
    if (control && --at<std::int32_t>(control,8)==0) {
        ControlVirtual0(control);
        if (--at<std::int32_t>(control,12)==0) ControlVirtual8(control,1);
    }
}
struct Array { void* data; std::int32_t count; std::int32_t capacity; };
// exec 142a7dd70. scratch is shared across element calls and service call;
// +8 holds the service-produced control pointer consumed during cleanup.
void OnRep_GliderOverride(Pawn* self, Frame* frame) {
    Finish(frame);
    void* stack = GliderStack(self);
    if (at<std::int32_t>(stack,8)==0) SetGliderClassNull(self);
    else if (Virtual6b0(self)) {
        void* service = FUN_141df62a0_UnresolvedArgs();
        Array names{nullptr,0,0};
        alignas(8) U8 scratch[16]; // initialization and exact returned object layout unresolved
        for (std::int32_t i=0; i<at<std::int32_t>(stack,8); ++i) {
            void* element = at<void*>(at<void*>(stack,0), i*8);
            if (element) {
                void* value = ElementVirtual400(element,scratch);
                U32 old = static_cast<U32>(names.count);
                ++names.count;
                if (names.capacity < names.count) Grow16_140690dd0(&names,old);
                std::memcpy(static_cast<U8*>(names.data)+static_cast<std::int32_t>(old)*16,value,16);
            }
        }
        U64 global = Global_14812b6e0();
        Array output{nullptr,0,0};
        // Decompiled storage has null pointer and a 32-bit value 1 at +8;
        // the remaining array header bytes were not initialized in this body.
        alignas(8) U8 filter[16];
        at<void*>(filter,0)=nullptr; at<U32>(filter,8)=1;
        Grow8_1406d93e0(filter,1);
        *at<U64*>(filter,0)=global;
        alignas(8) U8 pair[16]; at<U64>(pair,0)=0; at<U32>(pair,8)=0;
        ServiceVirtual330(service,scratch,&names,filter,&output,0,pair,0);
        Release(at<void*>(scratch,8));
        if (at<void*>(filter,0)) Free(at<void*>(filter,0));
        if (output.data) Free(output.data);
        if (names.data) Free(names.data);
    }
    Virtual1208(self);
}
extern U8* FacialTypeOverride(Pawn*); extern U8* GhostModeExitDuration(Pawn*);
extern void FUN_142345bd0_UnresolvedArgs(); extern void FUN_140dc67e0(void* sharedPair);
// exec 142a80d40. Offset labels are decompiler field anchors, not type claims.
void StopFacialAnimationStream(Pawn* self, Frame* frame) {
    Finish(frame);
    FacialTypeOverride(self)[0]=0;
    U8* anchor=GhostModeExitDuration(self);
    anchor[0x6c]=0;
    FUN_142345bd0_UnresolvedArgs();
    void** stored=reinterpret_cast<void**>(anchor+0x54);
    struct Pair { void* object; void* control; } local;
    local.object=*stored;
    if (local.object) {
        local.control=at<void*>(anchor,0x5c);
        if (local.control) ++at<std::int32_t>(local.control,8);
        FUN_140dc67e0(&local);
        if (reinterpret_cast<void*>(stored)!=reinterpret_cast<void*>(&local)) {
            *stored=nullptr;
            void* control=at<void*>(anchor,0x5c);
            if (control) { at<void*>(anchor,0x5c)=nullptr; Release(control); }
        }
    }
}
extern U8* CrouchMeshOffset(Pawn*); extern U8* VehicleInputStateUnreliable(Pawn*);
// exec 142a7e220, preserve field order, omission at source +10/dest +14.
void OnRep_VehicleInputStateUnreliable(Pawn* self, Frame* frame) {
    Finish(frame); U8* d=CrouchMeshOffset(self); U8* s=VehicleInputStateUnreliable(self);
    at<U32>(d,4)=at<U32>(s,0); at<U32>(d,8)=at<U32>(s,4);
    at<U32>(d,12)=at<U32>(s,8); at<U32>(d,16)=at<U32>(s,12);
    at<U32>(d,24)=at<U32>(s,20); at<U32>(d,28)=at<U32>(s,24);
    at<U64>(d,32)=at<U64>(s,28); at<U32>(d,40)=at<U32>(s,36);
}
// AITargetDied exec 142a7a940 only: helper is an unresolved native body.
extern void FUN_142301790(Pawn*, U64, U32, U64, U64, void*, U64, U64, void*);
void AITargetDied(Pawn* self, Frame* frame) {
    U64 momentum=0, hitComponent=0, hitLocation=0, instigatedBy=0, damage=0;
    U32 boneName[4]; boneName[0]=0;
    // Step outputs for P4 and P7 include opaque adjacent 32-bit stack words.
    // Their exact widths/ABI and any initialization by Step are unresolved.
    alignas(8) U8 causer[16], actor[16];
    Step(frame,&momentum); Step(frame,boneName); Step(frame,&hitComponent);
    Step(frame,&hitLocation); Step(frame,causer); Step(frame,&instigatedBy);
    Step(frame,&damage); Step(frame,actor);
    alignas(8) U8 actorPair[16], causerPair[16];
    at<U32>(actorPair,8)=at<U32>(actor,8);
    at<U32>(causerPair,8)=at<U32>(causer,8);
    at<U64>(actorPair,0)=at<U64>(actor,0);
    Finish(frame);
    at<U64>(causerPair,0)=at<U64>(causer,0);
    FUN_142301790(self,momentum,boneName[0],hitComponent,hitLocation,causerPair,instigatedBy,damage,actorPair);
}
extern U8* PawnGap8(Pawn*); extern U8* VehicleStateLocal(Pawn*);
extern U8* VehicleInputStateReliable(Pawn*); extern void* FUN_1428381e0_UnresolvedArgs();
extern void VehicleVirtual850(void*, void*); extern void FUN_1419297f0(void*, void*);
// implementation 142334ea0; input ABI and local struct extent unresolved.
void MulticastUpdateVehicleInputStateReliable_Implementation(Pawn* self, U8* input) {
    if (PawnGap8(self)[0xe8]==2) return;
    void* vehicle=at<void*>(VehicleStateLocal(self),0);
    U8 local[24]; local[0]=input[0];
    if (vehicle) {
        void* type=FUN_1428381e0_UnresolvedArgs();
        void* vehicleType=at<void*>(vehicle,16);
        auto index=at<std::int32_t>(type,0x38);
        if (index<=at<std::int32_t>(vehicleType,0x38) &&
            at<void*>(at<void*>(vehicleType,0x30),index*8)==static_cast<U8*>(type)+0x30)
            VehicleVirtual850(vehicle,local);
    }
    FUN_1419297f0(CrouchMeshOffset(self)+4,local);
    VehicleInputStateReliable(self)[0]=local[0];
}
} // namespace pawn_broad73
