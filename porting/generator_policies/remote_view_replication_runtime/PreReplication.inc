// CONFIDENCE: high
// Original 0x142344490. AFortPawn inherits the ACharacter implementation.
Super::PreReplication(ChangedPropertyTracker);
if (GetLocalRole() == ROLE_Authority)
{
    NativeUpdateAccelerationPack1423442e0();
    NativeUpdateRemoteViewData142344420();
    ReplicatedSkyTube = CurrentSkyTube;
    ReplicatedWaterBody = CurrentWaterBody;
    bReplicatedIsInVortex = bIsInVortex;
    bReplicatedIsInSlipperyMovement = bIsInSlipperyMovement || NativeHasSlipperyTags14232caf0();
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    bool bTimedOut = false;
    if (Movement->HasPredictionData_Server())
    {
        const FNetworkPredictionData_Server* Prediction = Movement->GetPredictionData_Server();
        bTimedOut = Prediction && Prediction->bTriggeringForcedUpdates && Prediction->bForcedUpdateDurationExceeded;
    }
    bIsProxySimulationTimedOut = bTimedOut;
}
// Original executes both following blocks on ALL roles.
DOREPLIFETIME_ACTIVE_OVERRIDE(AFortPlayerPawn, VortexParams, bReplicatedIsInVortex);
if (UFortAbilitySystemComponent* FortASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    const float PreviousMontagePosition = RepAnimMontageInfo.Position;
    const uint8 PreviousNextSection = RepAnimMontageInfo.NextSectionID;
    RepAnimMontageInfo = FortASC->NativeRepAnimMontageInfo1406ccd10();
    if (bNetMovementPrioritized && RepAnimMontageInfo.bRepPosition)
    {
        RepAnimMontageInfo.SkipPositionCorrection = false;
        // UCOMISS/JNZ: unordered positions compare as equal in the original.
        const bool bPositionChanged = (_mm_movemask_ps(_mm_cmpord_ss(
            _mm_set_ss(PreviousMontagePosition), _mm_set_ss(RepAnimMontageInfo.Position))) & 1) != 0 &&
            !_mm_ucomieq_ss(_mm_set_ss(PreviousMontagePosition), _mm_set_ss(RepAnimMontageInfo.Position));
        if (bPositionChanged || PreviousNextSection != RepAnimMontageInfo.NextSectionID)
            ForceNetUpdate();
    }
    LandingMontagePair = FortASC->LandingMontagePair;
}
