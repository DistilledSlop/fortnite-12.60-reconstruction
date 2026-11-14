#pragma once
#include "CoreMinimal.h"
#include "Misc/Crc.h"
#include "ABuildingSMActor.h"
#include <emmintrin.h>
namespace FortStructuralOriginal
{
inline int32 Bits(uint32 Value) { int32 R; FMemory::Memcpy(&R, &Value, 4); return R; }
inline int32 Add(int32 A, int32 B) { return Bits(uint32(A) + uint32(B)); }
inline int32 Mul(int32 A, int32 B) { return Bits(uint32(A) * uint32(B)); }
inline int32 HalfConvert(float V) {
 return _mm_cvtsi128_si32(_mm_srai_epi32(_mm_cvtsi32_si128(_mm_cvtss_si32(_mm_set_ss(V))), 1));
}
// Original three-int key hashes all 12 bytes with FCrc::MemCrc_DEPRECATED (141c8b630).
struct FGridKeyFuncs : TDefaultMapHashableKeyFuncs<FIntVector, TWeakObjectPtr<ABuildingSMActor>, false>
{
 static uint32 GetKeyHash(const FIntVector& V) { return FCrc::MemCrc_DEPRECATED(&V, 12); }
};
inline ABuildingSMActor* ValidBuilding(const TWeakObjectPtr<ABuildingSMActor>* Weak)
{
 ABuildingSMActor* A = Weak ? Weak->Get() : nullptr;
 return A && !A->IsPendingKill() && !A->bDestroyed ? A : nullptr;
}
}
