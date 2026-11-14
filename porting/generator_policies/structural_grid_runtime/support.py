"""Structural grid helpers: logical types, never original Windows object ABI."""
import hashlib
from pathlib import Path
IMAGE = Path('/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin')
HEADER = 'FortStructuralGrid.h'
PINS = [(5398561392, 32, '2d30528844f31f84f1792ded812366c7a678770c956a65e8c0f2c947a2c79d0a'), (5398562080, 32, 'cb54d74c21edb84b622140c3fe715134b028ddbadd518c0c20c789c2dade8c92'), (5388508592, 32, '230ac132529dae338606ac5e91798f7d039e7e89fa20e694021d8e5b7e6f8536'), (5398064096, 32, 'b15ba8b7c110ed17f9fe5f7b2d2e885eba857ce96c867a83a2d8a01cde875f8e'), (5398556224, 32, '21897661e5be18b1626340274634ef5296b682a8842149b36b66a87ec2dd86ba'), (5398584112, 32, 'e0ebf81b2e1b73562fecb12787c257e75697f65a432d13d85a753c9a679febe1'), (5398583776, 32, '869998cdaef4f61c63e8d041646b2c5fba48717c2ef12fa75b3a8e09c8523628'), (5398559552, 32, 'cc028502d75b5d18c132741326b86306aba22505c0645bd9aad42451180440f5'), (5398640176, 32, '883a1371b39ec861054c956531b3905a47ffbc69dab4876083ede3b27558e359'), (5415501120, 32, 'fb8b1181087604ff97e165545fbdcbd101808e8561b87f190fc7b42448cbc8ea'), (5398439680, 32, 'b602aabce736d83542719e4e161007b7ff5cea7f677b2c7219f2036a6befd913')]
TEXT = '#pragma once\n#include "CoreMinimal.h"\n#include "Misc/Crc.h"\n#include "ABuildingSMActor.h"\n#include <emmintrin.h>\nnamespace FortStructuralOriginal\n{\ninline int32 Bits(uint32 Value) { int32 R; FMemory::Memcpy(&R, &Value, 4); return R; }\ninline int32 Add(int32 A, int32 B) { return Bits(uint32(A) + uint32(B)); }\ninline int32 Mul(int32 A, int32 B) { return Bits(uint32(A) * uint32(B)); }\ninline int32 HalfConvert(float V) {\n return _mm_cvtsi128_si32(_mm_srai_epi32(_mm_cvtsi32_si128(_mm_cvtss_si32(_mm_set_ss(V))), 1));\n}\n// Original three-int key hashes all 12 bytes with FCrc::MemCrc_DEPRECATED (141c8b630).\nstruct FGridKeyFuncs : TDefaultMapHashableKeyFuncs<FIntVector, TWeakObjectPtr<ABuildingSMActor>, false>\n{\n static uint32 GetKeyHash(const FIntVector& V) { return FCrc::MemCrc_DEPRECATED(&V, 12); }\n};\ninline ABuildingSMActor* ValidBuilding(const TWeakObjectPtr<ABuildingSMActor>* Weak)\n{\n ABuildingSMActor* A = Weak ? Weak->Get() : nullptr;\n return A && !A->IsPendingKill() && !A->bDestroyed ? A : nullptr;\n}\n}\n'

def sources():
    with IMAGE.open('rb') as stream:
        for address, size, digest in PINS:
            stream.seek(address-0x140000000)
            if hashlib.sha256(stream.read(size)).hexdigest()!=digest:
                raise ValueError('structural-grid original image changed: '+hex(address))
    return {HEADER: TEXT}

def validate(ctx, snippets_by_class):
    users=[e for values in snippets_by_class.values() for e in values if HEADER in (e.get('includes') or [])]
    if not users: return
    for name in ('UBuildingStructuralSupportSystem','ABuildingLayoutRequirement','ABuildingSMActor'):
        if ctx['class_pkg'].get(name)!='/Script/FortniteGame':
            raise ValueError('structural grid dependency changed: '+name)
    allowed={('UBuildingStructuralSupportSystem', 'NativeCenterCellActors_0x178'): 'TMap<FIntVector, TWeakObjectPtr<ABuildingSMActor>, FDefaultSetAllocator, FortStructuralOriginal::FGridKeyFuncs> NativeCenterCellActors_0x178', ('UBuildingStructuralSupportSystem', 'NativeWallXActors_0x1c8'): 'TMap<int32, TWeakObjectPtr<ABuildingSMActor>> NativeWallXActors_0x1c8', ('UBuildingStructuralSupportSystem', 'NativeWallYActors_0x218'): 'TMap<int32, TWeakObjectPtr<ABuildingSMActor>> NativeWallYActors_0x218', ('UBuildingStructuralSupportSystem', 'NativeFloorActors_0x268'): 'TMap<int32, TWeakObjectPtr<ABuildingSMActor>> NativeFloorActors_0x268', ('UBuildingStructuralSupportSystem', 'NativeLayoutByUID_0x2b8'): 'TMap<uint32, TWeakObjectPtr<ABuildingLayoutRequirement>> NativeLayoutByUID_0x2b8', ('UBuildingStructuralSupportSystem', 'NativeWorldToGrid141c78270'): 'bool NativeWorldToGrid141c78270(const FVector& World, FIntVector& Out) const', ('UBuildingStructuralSupportSystem', 'NativeFindLayout141c78520'): 'ABuildingLayoutRequirement* NativeFindLayout141c78520(uint32 UID) const', ('UBuildingStructuralSupportSystem', 'NativeInsertLayout1412e1db0'): 'TWeakObjectPtr<ABuildingLayoutRequirement>& NativeInsertLayout1412e1db0(uint32 UID, const TWeakObjectPtr<ABuildingLayoutRequirement>& Actor)', ('UBuildingStructuralSupportSystem', 'NativeFindCenter141c76e40'): 'ABuildingSMActor* NativeFindCenter141c76e40(const FIntVector& Cell) const', ('UBuildingStructuralSupportSystem', 'NativeWallIndex141c7db30'): 'int32 NativeWallIndex141c7db30(const FIntVector& C, uint8 Side) const', ('UBuildingStructuralSupportSystem', 'NativeFindWall141c7d9e0'): 'ABuildingSMActor* NativeFindWall141c7d9e0(const FIntVector& Cell, uint8 Side) const', ('UBuildingStructuralSupportSystem', 'NativeFindFloor141c77b40'): 'ABuildingSMActor* NativeFindFloor141c77b40(const FIntVector& Cell, uint8 Below) const', ('ABuildingLayoutRequirement', 'NativeResourceMatches141bfebe0'): 'bool NativeResourceMatches141bfebe0(const ABuildingSMActor* Actor) const'}
    for entry in users:
        key=(entry.get('class'),entry.get('function'))
        if key not in allowed or entry.get('signature') != allowed[key]:
            raise ValueError('unrecognized structural grid support user: '+str(key))
