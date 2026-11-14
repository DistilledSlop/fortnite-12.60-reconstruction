// CONFIDENCE: HIGH. Complete original body; broader lifecycle remains open.
// Original 0x1420d8430, movement primary vtable +0x890.
if (MovementMode == MOVE_Walking)
{
    if (const AFortPlayerPawn* FortPawn = Cast<AFortPlayerPawn>(CharacterOwner))
    {
        if (FortPawn->bIsInSlipperyMovement)
        {
            APhysicsVolume* Volume = GetPhysicsVolume();
            if (Volume && (Volume->IsA<AWaterVolume_Deep>() || Volume->IsA<AWaterVolume_Shallow>()))
            {
                const float MinimumBraking = FortPawn->SlipperySlopeParams.BrakingDecelerationInWater.GetValueAtLevel(0.0f, nullptr);
                if (BrakingDeceleration < MinimumBraking)
                    BrakingDeceleration = MinimumBraking;
            }
        }
    }
}
Super::ApplyVelocityBraking(DeltaTime, Friction, BrakingDeceleration);
