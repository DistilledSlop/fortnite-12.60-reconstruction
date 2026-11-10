// CONFIDENCE: MEDIUM - exec 0x142758050 dispatches through vtable slot 0xa90 to 0x141c1bf20 (ABuildingSMActor::SetMirrored body).
// Original addresses: 0x142758050 (exec thunk) ; 0x141c1bf20 (virtual body, vtable slot 0xa90)
// bMirrored is bit 2 of the bool bitfield at +0x6a8. Mirroring negates the X component of the StaticMeshComponent relative scale.
// The engine scale notification on the component (vtable slot 0x4e0, identity unresolved) is performed by SetRelativeScale3D; see deps.
if (bMirrored == bIsMirrored)
{
    return;
}
bMirrored = bIsMirrored;
if (UStaticMeshComponent* Mesh = StaticMeshComponent)
{
    FVector Scale = Mesh->GetRelativeScale3D();
    Scale.X = bIsMirrored ? -FMath::Abs(Scale.X) : FMath::Abs(Scale.X);
    Mesh->SetRelativeScale3D(Scale);
    RefreshComponentRenderStates();
    ReplicatedDrawScale3D = Mesh->GetRelativeScale3D();
}
