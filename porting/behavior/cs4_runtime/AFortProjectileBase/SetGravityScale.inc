// Behaviour snippet: AFortProjectileBase::SetGravityScale (body only; spliced into the generated member function).
// Original body: 0x1426df650 (thunk 0x142aa6840).
// Evidence: AActor::Role (+0xf0) == ROLE_Authority gate; virtual slot +0x658 = AFortGameplayEffectDeliveryActor::NativeHelper1426a5ae0 must be false; stores GravityScale (+0x5fc, reflected) and UFortProjectileMovementComponent ProjectileGravityScale (+0x10c via ProjectileMovementComponent +0x640).
// CONFIDENCE: MEDIUM
if (GetLocalRole() != ROLE_Authority)
{
    return;
}
if (NativeHelper1426a5ae0())
{
    return;
}
GravityScale = InitialGravityScale;
static_assert(sizeof(UFortProjectileMovementComponent) > 0, "UFortProjectileMovementComponent must be complete for the store");
ProjectileMovementComponent->ProjectileGravityScale = InitialGravityScale;
