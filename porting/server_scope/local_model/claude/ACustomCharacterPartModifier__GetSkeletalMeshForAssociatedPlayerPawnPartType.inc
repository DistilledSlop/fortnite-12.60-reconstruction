// CONFIDENCE: HIGH - exec 0x142779450 calls body 0x141da6f00. The pawn comes from the weak pointer (virtual 0x628 -> 0x141da5180),
// Original addresses: 0x142779450 (exec thunk) ; 0x141da6f00 (direct body)
// the part from GetCharacterPartType (virtual 0x640 -> 0x1412d96f0), and the mesh from the pawn (virtual 0x1110 -> 0x1423240c0).
AFortPlayerPawn* Pawn = FortPlayerPawnPtr.Get();
if (Pawn == nullptr)
{
    return nullptr;
}
return Pawn->GetCharacterPartSkeletalMeshComponent(GetCharacterPartType());
