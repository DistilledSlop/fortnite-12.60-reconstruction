// CONFIDENCE: HIGH. Stores bEnableBlockingCollisionWithOtherAI and, on the authority, sets the capsule response for the AI pawn channel (block when enabled, ignore otherwise).
// Original address: 0x1427a5a40 (exec). Native body: 0x1422ec1a0 (flag bit 0x40000; capsule virtual slot 0x800 on CapsuleComponent with channel 0x11).
if (bInEnableBlockingCollisionWithOtherAI)
{
    AIFlags1344 |= 0x40000;
}
else
{
    AIFlags1344 &= ~0x40000;
}

if (Role == ROLE_Authority && CapsuleComponent != nullptr)
{
    CapsuleComponent->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(0x11), bInEnableBlockingCollisionWithOtherAI ? ECR_Block : ECR_Ignore);
}
