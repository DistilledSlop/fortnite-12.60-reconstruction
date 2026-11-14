// CONFIDENCE: high
// Original 0x1420df7e0. Native declarations/state proposals accompany this file.
if (MovementMode == MOVE_Custom)
{
    if (CustomMovementMode == 3)
    {
        // Original calls this predicate although both arms select the same owner.
        (void)NativeAirDirection_1420e4740();
        const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
        check(Pawn); // Original directly dereferences the Fort pawn contract here.
        if (Pawn->CurrentSkyTube) return ParachuteControlParamsSkydive;
        if (Pawn->bIsInVortex) return VortexParachuteControlParams;
        if (Pawn->bLocalInGliderRedeploy) return ParachuteControlParamsGliderRedeploy;
        return Pawn->bLocalIsSkydiving ? ParachuteControlParamsSkydive : ParachuteControlParamsJump;
    }
    if (CustomMovementMode == 4)
    {
        const bool bActive = NativeAirDirection_1420e4ab0();
        const AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(CharacterOwner);
        check(Pawn);
        if (!bActive)
        {
            if (Pawn->CurrentSkyTube) return SkyTubeSkydivingControlParamsPassive;
            return Pawn->bIsInVortex ? VortexSkydivingControlParamsPassive : SkydivingControlParamsPassive;
        }
        if (!Pawn->CurrentSkyTube && !Pawn->bIsInVortex) return SkydivingControlParamsActive;
        const float AccelerationZ = NativeAirAcceleration().Z;
        // COMISS/CMOVB selects the active block for negative or unordered Z.
        if (Pawn->CurrentSkyTube)
            return !(AccelerationZ >= 0.0f) ? SkyTubeSkydivingControlParamsActive : SkyTubeSkydivingControlParamsActiveUpward;
        return !(AccelerationZ >= 0.0f) ? VortexSkydivingControlParamsActive : VortexSkydivingControlParamsActiveUpward;
    }
    if (CustomMovementMode == 9) return BalloonControlParams;
    if (CustomMovementMode == 12) return FloatingControlParams;
}
// Original has a thread-safe local static with ctor 0x1420d2af0. Mode 5 reaches it.
static const FAirControlParams DefaultParams = [] {
    FAirControlParams Result;
    Result.MaxAcceleration = FScalableFloat(0.0f);
    Result.LateralFriction = FScalableFloat(0.0f);
    Result.MaxLateralSpeed = FScalableFloat(0.0f);
    Result.TerminalVelocity = FScalableFloat(0.0f);
    Result.UpwardTerminalVelocity = FScalableFloat(0.0f);
    Result.GravityScalar = FScalableFloat(0.0f);
    Result.CustomGravityCeiling = FScalableFloat(0.0f);
    Result.CustomGravityCeilingWidth = FScalableFloat(0.0f);
    return Result;
}();
return DefaultParams;
