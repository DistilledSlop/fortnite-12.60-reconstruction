// CONFIDENCE: MEDIUM. Stores the weapon collision component, copies its two tuning values, then refreshes the capsule collision responses for the weapon state.
// Original address: 0x1427a65b0 (exec). Native body: 0x1422ed360; state refresh 0x1422f2450 (inlined below).
if (InWeaponCollisionComponent == nullptr || Mesh == nullptr)
{
    return;
}

WeaponCollisionComponent = InWeaponCollisionComponent;
WeaponCollisionTuningA = InWeaponCollisionComponent->TuningA; // source +0x13c
WeaponCollisionTuningB = InWeaponCollisionComponent->TuningB; // source +0x124

// Refresh (0x1422f2450): capsule responses for the weapon state.
const bool bFlagsSet = (ProtectedFlags521 & 0x02) != 0 && (ProtectedFlags521 & 0x04) != 0;
if (bFlagsSet && WeaponCollisionComponent == nullptr && CapsuleComponent != nullptr && CapsuleBoundsDifferFromSentinel(CapsuleComponent))
{
    CapsuleComponent->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(0x14), ECR_Block);
    CapsuleComponent->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(0x12), ECR_Overlap);
}
if (WeaponCollisionComponent == nullptr)
{
    CapsuleComponent->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(0x10), ECR_Block);
}
