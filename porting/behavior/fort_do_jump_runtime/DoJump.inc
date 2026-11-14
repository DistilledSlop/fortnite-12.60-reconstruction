// CONFIDENCE: HIGH. Complete base Fort body 0x1420dc080.
// Athena override is independently incomplete.
const uint8 PreviousFlag = NativeDoJumpFlag1dfd;
NativeDoJumpFlag1dfd = 1;
const float PreviousJumpZ = JumpZVelocity;
JumpZVelocity = PreviousJumpZ * 1.05f;
if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
    if (Player->bIsInSlipperyMovement)
    {
        JumpZVelocity *= Player->SlipperySlopeParams.SlopeLaunchVerticalVelocityBoostMultiplierJumping
            .GetValueAtLevel(0.0f, nullptr);
        if (IsMovingOnGround() && CurrentFloor.IsWalkableFloor())
        {
            const FVector ImpactNormal = CurrentFloor.HitResult.ImpactNormal;
            if (ImpactNormal.Z > 0.0001f)
                JumpZVelocity += NativeComputeSlipperyJump1420da5d0(Velocity, ImpactNormal, *Player).Z;
        }
    }
}
const bool bResult = UCharacterMovementComponent::DoJump(bReplayingMoves);
JumpZVelocity = PreviousJumpZ;
if (bResult)
    FallingStartedZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : FLT_MAX;
NativeDoJumpFlag1dfd = PreviousFlag;
return bResult;
